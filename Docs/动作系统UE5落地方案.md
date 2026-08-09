# UE5 动作系统落地方案

> 状态：当前实现基线
>
> 目标：在 ActionRPG/GAS 基础上，建立支持鬼泣、绝区零式连段、派生、取消和预输入的单机动作系统。
>
> 本文记录已经确定的第一版方案、明确的非目标，以及后续扩展方向。网络联机暂不实现，但第一版保留动作帧编号和输入序号，避免以后无法验证和重放。

## 1. 设计目标与边界

### 1.1 第一阶段目标

第一阶段先完成一个可测试的垂直切片：

```text
Idle
  -> Light01
       -> Light02                 普通连段
       -> Dodge                    取消
       -> ForwardSlash              前冲斩
       -> BackCounter               后撤反击
```

必须支持：

- 60Hz 动作逻辑帧。
- 输入事件采集和 RingBuffer 输入历史。
- 以终结按钮为终点的基础搓招识别。
- ActionPattern 意图的预输入缓存。
- 优先级和同优先级后输入覆盖。
- ActionWindow 配置和运行时 ActionTransition。
- ActionInfo 静态配置和 ActionInfoSpec 运行实例。
- 通过 GAS 激活动作 Ability，并播放对应 Montage。

### 1.2 明确不在第一阶段实现

- 网络预测、服务器验证和回滚。
- 复杂的逐步骤 MaxGap、可选步骤和输入容错。
- 可变输入优先级 Channel 系统。
- 完整动作图编辑器。
- 通过 AnimNotify 驱动核心取消或伤害规则。
- 逐逻辑帧调整 Montage PlayRate。

这些能力不是不需要，而是应该建立在第一版的输入、动作时间和转换模型稳定之后。

## 2. 核心设计决策

### 2.1 动作逻辑采用固定 60Hz

动作系统使用自己的固定步进：

```text
ActionSimulationRate = 60Hz
ActionStep = 1 / 60 秒
```

这代表“动作第 20 帧”是动作时钟达到 `20 / 60` 秒，而不是第 20 次 UE Tick、动画采样或渲染帧。

`UActionComponent` 在普通 GameTick 中使用 accumulator：

```cpp
Accumulator += DeltaSeconds;

while (Accumulator >= ActionStep)
{
    SimulateOneActionFrame();
    Accumulator -= ActionStep;
}
```

因此：

```text
动作逻辑：固定 60Hz
UE GameThread：可变 Tick
动画 Montage：按时间播放
渲染：按实际帧率
```

如果一帧掉帧，动作系统可以执行多个动作步进；如果渲染帧率高于 60Hz，则部分渲染帧不会产生新的动作步进。

为了避免掉帧时无限追赶，第一版应设置 `MaxActionStepsPerTick`。超过上限时记录警告，并让动作时间正常推进，而不是让整个游戏进入无限追帧。

### 2.2 动画采样率与动作逻辑率分离

项目可以将动画资产的目标采样率规范为 60Hz：

```text
ActionDesignRate = 60Hz
AnimationSourceRate = 60Hz
AnimationEvaluationRate = EngineDriven
```

`AnimationSourceRate` 只代表动画资产的制作和采样规范，不能保证动画运行时每秒评估 60 次。Montage 仍然基于播放时间运行，动作窗口由 ActionSystem 的动作帧判断。

### 2.3 ActionPattern 是动作入口配置

`ActionInfo` 可以拥有一个或多个入口 Pattern：

```text
ActionInfo
    ActionTag
    Montage
    DurationFrames
    EntryPatterns[]
    ActionWindows[]
```

这属于组合关系，而不是继承关系。

`FInputPattern` 描述玩家如何表达一个动作意图，`ActionInfo.EntryPatterns` 描述这个动作允许从哪些输入方式进入。未来同一个动作可以通过多个 Pattern、命中派生、Gameplay Event 或 AI 指令进入。

运行时应提前建立 Pattern 索引，而不是每个动作帧扫描所有动作资产：

```text
终结输入 GameplayTag
    -> 可能匹配的 InputPattern
    -> PatternRecognizer
```

### 2.4 ActionWindow 编译为运行时 Transition

`ActionWindow` 是面向策划的时间轴配置，解决“什么时候允许转换”的问题。

```text
Light01
    [20, 30) Window.CanCombo
        Pattern.Attack -> Light02

    [27, 35) Window.DodgeCancel
        Pattern.Dodge -> Dodge
```

加载 `ActionInfo` 时，将 ActionWindow 编译为运行时可查询的转换规则：

```text
FActionTransition
    SourceActionTag
    TargetActionTag
    RequiredPatternTag
    StartFrame
    EndFrame
    Priority
    Conditions
    TransitionMode
```

概念上仍然保留两个层次：

```text
ActionWindow       = When，时间门和窗口语义
ActionTransition   = Where，目标动作和转换规则
```

实现上可以把 TransitionRule 嵌套在 ActionWindow 内，或者在加载时生成独立的 Transition 数组。第一版建议采用“ActionWindow 配置、运行时编译 Transition”的方式。

### 2.5 ActionInfo 与 ActionInfoSpec

保留当前命名：

```text
ActionInfo       = 静态动作配置
ActionInfoSpec   = 一次动作的运行实例
```

`ActionInfo` 适合实现为 `UDataAsset` 或 `UPrimaryDataAsset`，保存：

- ActionTag。
- GameplayAbilityClass。
- Montage 和可选 Section。
- TotalFrames。
- EntryPatterns。
- ActionWindows。
- 动作表现和战斗 Profile 的引用。

`ActionInfoSpec` 只保存运行时状态：

- ActionInfo 引用。
- ActionInstanceId。
- CurrentActionFrame。
- ActionTime。
- 当前 ActiveWindowTags。
- 当前 Montage 播放句柄或运行状态。

静态窗口不应在每次动作实例化时完整复制。

## 3. UE5 中的模块职责

第一版暂时放在现有 `ActionRPG` Runtime Module 中，避免过早拆模块。建议的类型职责如下：

```text
UActionComponent
    固定动作步进、输入历史、PatternRecognizer、预输入、ActionInfoSpec

UActionInfo / FActionWindow / FInputPattern
    静态数据和编辑器配置

FInputEvent / FCombatCommand / FBufferedActionIntent
    运行时输入数据

FActionTransition
    编译后的动作转换查询数据

UGameplayAbility
    动作执行、资源消耗、GameplayTag、GameplayEffect、GameplayCue、AbilityTask
```

Enhanced Input 只负责底层输入映射和基础 Trigger：

```text
Enhanced Input Action
    -> FInputEvent
    -> ActionComponent
```

不要把完整的搓招历史和动作转换逻辑塞进 Enhanced Input Trigger。Enhanced Input 支持 Chorded Action、上下文优先级和自定义 Trigger，适合输入入口；PatternRecognizer 才负责跨帧顺序匹配。[Enhanced Input 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/enhanced-input-in-unreal-engine)

## 4. 输入与搓招模型

### 4.1 FInputEvent

```text
FInputEvent
    InputSequence
    CapturedTimeSeconds
    CapturedCombatFrame
    InputTag
    EventType
    AnalogValue
```

`EventType` 第一阶段至少支持：

```text
Pressed
Released
AxisChanged
DirectionEntered
```

按钮和方向的“按住状态”不等于“按下事件”。搓招主要匹配边沿事件，动作状态窗口则可以查询当前持有状态。

### 4.2 InputHistory

使用 RingBuffer 保存最近 N 个 CombatFrame 的输入：

```text
InputHistory
    Frame 100 -> Direction + Pressed/Released Events
    Frame 101 -> Direction + Pressed/Released Events
    Frame 102 -> Direction + Pressed/Released Events
```

每个动作帧可以保存方向快照和多个按钮事件。第一阶段不依赖严格的物理按键到达顺序；同一个 CombatFrame 内的按钮组合按照预先定义的规则判断。

### 4.3 FInputPatternStep 与 FInputPattern

第一阶段的 Step 类型：

```text
DirectionEntered
ButtonPressed
```

```text
FInputPattern
    PatternTag
    Steps[]
    MaxDurationFrames
    Priority
    Specificity
    TerminalInputTag
```

示例：

```text
Pattern.ForwardSlash
    Direction.Forward
    Button.Attack
    MaxDurationFrames = 8

Pattern.BackCounter
    Direction.Back
    Direction.Forward
    Button.Attack
    MaxDurationFrames = 8
```

第一版以终结输入为匹配终点，向前扫描 InputHistory。未来可以将匹配器改为编译后的有限状态机，但不改变 `FInputPattern` 的数据含义。

### 4.4 FCombatCommand 与预输入

PatternRecognizer 匹配成功后生成 `FCombatCommand`。它表示：

```text
玩家已经表达了某个动作意图
```

它不代表最终 Action 已经确定。

预输入项需要保存：

```text
PatternTag
Priority
DetectedFrame
ExpireFrame
InputSequence
```

预输入规则：

1. 不同优先级的意图可以同时存在。
2. 同优先级只保留最后进入的意图。
3. 查询时取最高优先级的未过期意图。
4. 当前窗口尚未允许执行时保留意图。
5. 成功执行后消费意图。
6. 过期、明确失败或被新意图覆盖时删除。

第一阶段不引入多个 Channel。若以后出现“攻击和闪避必须分别缓存”的需求，再引入 BufferGroup，而不是预先为所有输入建立复杂 Channel 层。

## 5. ActionComponent 的固定动作循环

```text
普通 GameTick(DeltaSeconds)
    -> 累积真实时间
    -> 产生 0..N 个 ActionStep
```

每个 `ActionStep`：

```text
1. 分配本次 CombatFrameId
2. 接收并归档本帧 FInputEvent
3. 更新 InputHistory
4. 运行 PatternRecognizer
5. 写入或更新 PreInputBuffer
6. 更新当前 ActionInfoSpec.CurrentActionFrame
7. 激活当前帧的 ActionWindow
8. 收集匹配的 ActionTransition
9. 检查动作状态和附加条件
10. 按优先级选择唯一 Transition
11. 消费输入并提交新的 ActionInfoSpec
12. 通知 GAS 和动画表现层
```

窗口使用 `[StartFrame, EndFrame)`，例如 `[20, 30)` 表示动作帧 20 到 29。

## 6. GAS 与 Montage 的落地边界

动作系统选择“做什么”，GAS 负责“如何执行游戏效果”：

```text
ActionTransition
    -> TryActivateAbility
    -> Ability Commit Cost / Cooldown
    -> Play Montage
    -> AbilityTask / GameplayEvent
    -> HitSystem / GameplayEffect / GameplayCue
```

GAS 负责 Ability 生命周期、资源消耗、GameplayTag、GameplayEffect、GameplayCue 和 AbilityTask。ActionComponent 负责动作时钟、输入意图和转换选择。Lyra 的 Ability Set、Input Tag 激活和 Ability Activation Group 也体现了“输入/能力授予/能力执行”分层的思路。[Lyra Abilities](https://dev.epicgames.com/documentation/unreal-engine/abilities-in-lyra-in-unreal-engine?lang=en-US)

Montage 负责动画表现、Sections、Slots、Blend 和 Root Motion。核心取消窗口、无敌窗口和命中窗口由 ActionWindow/ActionTransition 维护，不以客户端 AnimNotify 作为唯一事实来源。

AnimNotify 可以继续用于：

- 音效。
- 粒子和武器拖尾。
- 镜头震动。
- 动画表现标记。
- 非关键 Gameplay Event。

UE 官方将 Montage Notify 区分为 Queued 和 Branching Point，后者更精确但成本更高；这也说明 Notify 是动画时间轴事件，不应自动等同于动作系统的固定逻辑帧。[Animation Montage](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-montage-in-unreal-engine?lang=en-US)、[Animation Notifies](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-notifies-in-unreal-engine?lang=en-US)

## 7. 第一阶段实施清单

### Phase 1：固定动作时间

- 创建 `UActionComponent`。
- 实现 60Hz accumulator。
- 实现 `CombatFrameId`。
- 实现 ActionInfoSpec 的创建、推进和结束。
- 实现窗口 `[StartFrame, EndFrame)` 查询。

### Phase 2：输入闭环

- 接入 Enhanced Input。
- 定义 `FInputEvent`。
- 实现方向八方向量化和 `DirectionEntered`。
- 实现 RingBuffer InputHistory。
- 实现 Attack、Dodge、Forward、Back 等基础输入事件。

### Phase 3：搓招与预输入

- 实现 `FInputPatternStep` 和 `FInputPattern`。
- 实现终结按钮倒推匹配。
- 实现 `FCombatCommand`。
- 实现按优先级分组的预输入缓存。
- 实现同优先级后输入覆盖。
- 实现唯一 Transition 选择和输入消费。

### Phase 4：动作与 GAS

- 创建 ActionInfo DataAsset。
- 将 ActionWindow 编译为 ActionTransition。
- 建立 ActionTag 与 GameplayAbilityClass 的映射。
- 将 Transition 接入 ASC `TryActivateAbility`。
- 使用现有 Montage AbilityTask 播放动画。
- 用 GameplayEffect、GameplayCue 和命中系统完成攻击闭环。

## 8. 后续扩展路线

### 8.1 输入匹配扩展

- 每个 Step 独立 `MaxGapFrames`。
- Hold、Release、Chord、Charge。
- Optional Step 和 Forbidden Input。
- 斜方向和摇杆死区容错。
- Pattern 编译为有限状态机。
- 输入录制、回放和训练场调试。

### 8.2 动作系统扩展

- Immediate、Queued、Forced 等 TransitionMode。
- 普通取消、命中取消、受击取消、落地取消。
- 动作派生树和动作图编辑器。
- 动作变体、武器变体和角色专属 Pattern。
- Hit Confirm、Just Frame、Perfect Dodge。
- Hit Stop、Slow Motion、Time Dilation。
- Root Motion、转向、位移和目标吸附 Profile。

### 8.3 动画与工具扩展

- 将 ActionWindow 做成 Montage 风格时间轴编辑器。
- 从 Montage Marker 或 Notify 生成窗口初始数据。
- 编译期检查 ActionInfo 总帧数与 Montage 时长是否一致。
- 显示当前 ActionFrame、ActiveWindow、输入历史和候选 Transition。
- 录制“输入 -> Pattern -> Transition -> Ability”的完整调试轨迹。

### 8.4 网络扩展

暂不实现，但保留：

- CombatFrameId。
- InputSequence。
- ActionInstanceId。
- PatternTag。
- ActionTransition 的确定性输入。

未来可采用客户端预测、服务器验证和最小 Action Snapshot。UE 的 Network Prediction 已经提供 Fixed Tick、输入命令批量发送、Fixed Tick Smoothing 等基础概念；GAS 则负责 Ability、Effect、Tag 和预测相关能力。[UE Network Prediction](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Plugins/NetworkPrediction)、[GGPO SDK](https://github.com/pond3r/ggpo)、[Mortal Kombat / Injustice Rollback 分享](https://www.gdcvault.com/play/1025021/8-Frames-in-16ms-Rollback)

## 9. 验证标准

第一版至少要验证：

- 30、60、120 FPS 下动作逻辑帧数和动作时长一致。
- 一帧跨过多个逻辑帧时，输入不会重复消费。
- `[20, 30)` 的窗口只在帧 20 到 29 生效。
- 同优先级意图后输入覆盖前输入。
- 不同优先级意图选择最高优先级。
- 预输入过期后不会执行。
- Y+B 同一 CombatFrame 才能形成组合。
- ForwardSlash 和 BackCounter 能正确区分方向顺序。
- Transition 只提交一次，不会在一个逻辑帧触发多个动作。
- ActionInfoSpec 结束后不会继续读取旧窗口。
- GAS Ability 激活失败时，预输入不会被错误消费。

## 10. 方案依据与参考资料

### Unreal Engine 官方资料

- [Enhanced Input](https://dev.epicgames.com/documentation/en-us/unreal-engine/enhanced-input-in-unreal-engine)：Input Action、Mapping Context、Trigger、Chord 和自定义输入处理。
- [Animation Montage](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-montage-in-unreal-engine?lang=en-US)：Montage、Sections、Slots、时间轴和运行时 Section 控制。
- [Animation Notifies](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-notifies-in-unreal-engine?lang=en-US)：Notify Window、Queued 与 Branching Point 的精度差异。
- [Gameplay Ability System](https://dev.epicgames.com/documentation/en-us/unreal-engine/gameplay-ability-system-for-unreal-engine?lang=en-US)：Ability、AbilityTask、Attribute、GameplayEffect 和 GameplayCue 的职责。
- [Using Gameplay Abilities](https://dev.epicgames.com/documentation/en-us/unreal-engine/using-gameplay-abilities-in-unreal-engine)：Ability 生命周期、成本、动画、分支和预测能力。
- [Lyra Sample Game](https://dev.epicgames.com/documentation/en-us/unreal-engine/lyra-sample-game-in-unreal-engine?lang=en-US)：模块化 UE5 样例、输入和 GAS 组织方式。
- [Abilities in Lyra](https://dev.epicgames.com/documentation/unreal-engine/abilities-in-lyra-in-unreal-engine?lang=en-US)：Input Tag、Ability Set、Activation Group 和 Tag Relationship。
- [Network Prediction API](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Plugins/NetworkPrediction)：Fixed Tick 与 Independent/Variable Tick 的 UE 基础设施。
- [Physics Sub-Stepping](https://dev.epicgames.com/documentation/en-us/unreal-engine/physics-sub-stepping-in-unreal-engine)：UE 可变帧率与固定子步进的关系。
- [Action RPG Sample](https://dev.epicgames.com/documentation/en-us/unreal-engine/action-rpg-game?application_version=4.27)：本项目所基于的 GAS Action RPG 示例说明。

### 成熟动作与格斗方案

- [Ikemen-GO](https://github.com/ikemen-engine/Ikemen-GO)：公开的成熟格斗游戏引擎，支持 MUGEN 资源和命令系统。
- [Ikemen-GO Command API](https://github.com/ikemen-engine/Ikemen-GO/wiki/Lua#commandadd)：公开定义了命令输入时间、预输入缓冲时间、暂停期间缓冲和步骤粒度。
- [GGPO](https://github.com/pond3r/ggpo)：成熟格斗游戏回滚网络 SDK，适合作为未来网络阶段的参考。
- [Fix Your Timestep](https://gafferongames.com/post/fix_your_timestep/)：固定步进、accumulator、模拟与渲染解耦的经典资料。

### 动作游戏官方技术分享

- [8 Frames in 16ms: Rollback Networking in Mortal Kombat and Injustice 2](https://media.gdcvault.com/gdc2018/presentations/Stallone_Michael_8FramesIn16ms.pdf)：NetherRealm 的 Hard 60Hz、确定性模拟和帧级回滚实践。
- [Evolving God of War's Combat for a New Perspective](https://media.gdcvault.com/gdc2019/presentations/Sheth_Mihir_EvolvingCombat.pdf)：Santa Monica Studio 对动作节奏、目标、镜头、攻击反馈和玩家意图的公开分享。
- [Breaking Barriers: Combat Accessibility in God of War Ragnarök](https://www.gdcvault.com/play/1028726/Breaking-Barriers-Combat-Accessibility-in)：Santa Monica Studio 关于动作游戏战斗可读性、可访问性和战斗设计取舍的分享。

这些公开资料不能证明鬼泣或绝区零内部就是同一种实现，但可以支持本方案的几个关键判断：动作逻辑应有清晰的时间基准，输入识别应独立于最终动作执行，动作数据应数据驱动，动画表现与战斗规则应保持边界，GAS 应承担能力和效果层而不是完整的搓招解析器。

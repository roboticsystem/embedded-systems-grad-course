---
number headings: first-level 2, start-at 14
---

## 14 第14章 机器人导航与路径规划

自主导航是移动机器人最核心的能力——在已知或未知环境中，从起点安全、高效地到达目标点。本章从路径规划的基本算法出发，逐步深入到 ROS2 Nav2 导航栈的工程实践，使学生具备为轮式机器人实现完整自主导航的能力。

学习目标：

- 掌握经典路径规划算法（A\*、Dijkstra、RRT）的原理与实现；
- 理解局部规划与避障算法（DWA、TEB）；
- 了解代价地图的构建与使用；
- 掌握 ROS2 Nav2 导航栈的架构与配置；
- 能完成从地图加载到自主导航的完整流程。

---

### 14.1 导航问题概述

#### 14.1.1 自主导航的要素

一个完整的自主导航系统需要回答四个问题：

1. **我在哪？**（定位）——通过 SLAM 或已知地图上的定位（AMCL）；
2. **要去哪？**（目标）——用户指定或任务规划给出的目标位姿；
3. **怎么去？**（全局规划）——在地图上规划从当前位置到目标的路径；
4. **如何走？**（局部规划）——跟踪全局路径、实时避障、控制速度。

```bob
┌──────────────────────────────────────────────────────┐
│                自主导航系统模块                        │
│                                                      │
│  ┌──────────┐                  ┌──────────────┐      │
│  │  定位    │                  │  目标输入    │      │
│  │ AMCL     │                  │  用户/任务   │      │
│  └────┬─────┘                  └──────┬───────┘      │
│       │                               │              │
│       └──────────┬────────────────────┘              │
│                  │                                   │
│           ┌──────▼───────┐                           │
│           │  全局规划    │                           │
│           │ "A* / Dijkstra"│                           │
│           └──────┬───────┘                           │
│                  │ 全局路径                           │
│           ┌──────▼───────┐                           │
│           │  局部规划    │                           │
│           │ "DWA / TEB"  │◄──── 传感器实时数据       │
│           └──────┬───────┘                           │
│                  │ 速度指令                           │
│           ┌──────▼───────┐                           │
│           │  运动控制    │                           │
│           │ "cmd_vel"    │                           │
│           └──────────────┘                           │
└──────────────────────────────────────────────────────┘
```

#### 14.1.2 导航与前置章节的关系

| 依赖章节 | 提供的能力 | 在导航中的角色 |
|---------|-----------|--------------|
| 第8章 机器人运动与电机驱动 | 差速运动学、PWM 控制 | 执行速度指令 |
| 第9章 闭环控制与 PID | 编码器反馈、速度闭环 | 底层速度控制 |
| 第13章 ROS2 程序设计 | 节点、话题、服务、micro-ROS | 通信框架 |
| 第14章 SLAM | 地图构建与定位 | 提供地图和位姿 |

---

### 14.2 全局路径规划算法

全局规划器在整个地图上搜索一条从起点到终点的无碰撞路径。

#### 14.2.1 图搜索基础

路径规划问题可建模为图搜索：将地图离散化为节点（网格或采样点），相邻节点之间建立边（代价为距离或通行难度）。

**Dijkstra 算法**：

- 找到起点到所有节点的最短路径
- 不使用启发信息，向所有方向均匀扩展
- 时间复杂度 $O((V+E)\log V)$（优先队列实现）
- 保证最优解

```
Dijkstra(start, goal, graph):
  dist[start] = 0
  open_set = PriorityQueue({start: 0})
  
  while open_set is not empty:
    current = open_set.pop_min()
    if current == goal: return path
    
    for neighbor in graph.neighbors(current):
      new_cost = dist[current] + edge_cost(current, neighbor)
      if new_cost < dist[neighbor]:
        dist[neighbor] = new_cost
        parent[neighbor] = current
        open_set.push(neighbor, new_cost)
  
  return failure
```

#### 14.2.2 A\* 算法

A\* 是机器人路径规划中使用最广泛的算法。它在 Dijkstra 的基础上引入启发函数 $h(n)$，优先搜索更可能接近目标的方向。

**核心公式**：

$$f(n) = g(n) + h(n)$$

- $g(n)$：从起点到节点 $n$ 的实际代价
- $h(n)$：从节点 $n$ 到目标的启发式估计代价
- $f(n)$：总估计代价

**启发函数的要求**：

- **可容纳性（Admissible）**：$h(n) \leq h^*(n)$，即不高估实际代价——保证找到最优解
- **一致性（Consistent）**：$h(n) \leq c(n,n') + h(n')$——保证不重复搜索

常用启发函数：

| 启发函数 | 公式 | 适用场景 |
|---------|------|---------|
| 曼哈顿距离 | $\|x_1-x_2\| + \|y_1-y_2\|$ | 4-连通网格 |
| 欧几里得距离 | $\sqrt{(x_1-x_2)^2 + (y_1-y_2)^2}$ | 8-连通网格 |
| 切比雪夫距离 | $\max(\|x_1-x_2\|, \|y_1-y_2\|)$ | 8-连通网格 |

```bob
┌──────────────────────────────────────────────────────────┐
│           Dijkstra vs A* 搜索对比                         │
│                                                          │
│   Dijkstra（无启发）          A*（有启发）               │
│  ┌─────────────────┐     ┌─────────────────┐            │
│  │ . . . . . . . . │     │ . . . . . . . . │            │
│  │ . # # # # . . . │     │ . # # # # . . . │            │
│  │ . . . . # . . . │     │ . . . . # . . . │            │
│  │ S * * * # * * G │     │ S * * * # . . G │            │
│  │ . * * * # * * . │     │ . . * * # * * . │            │
│  │ . * * * * * * . │     │ . . . * * * . . │            │
│  │ . * * * * * . . │     │ . . . . . . . . │            │
│  │ . . . . . . . . │     │ . . . . . . . . │            │
│  └─────────────────┘     └─────────────────┘            │
│  扩展节点多，搜索慢         扩展节点少，搜索快            │
└──────────────────────────────────────────────────────────┘
```

**A\* Python 实现**：

```python
import heapq

def a_star(grid, start, goal):
    """A* 路径规划算法
    grid: 2D 数组，0=可通行，1=障碍物
    start, goal: (row, col) 元组
    """
    rows, cols = len(grid), len(grid[0])
    
    # 启发函数：欧几里得距离
    def heuristic(a, b):
        return ((a[0]-b[0])**2 + (a[1]-b[1])**2) ** 0.5
    
    # 8-连通方向
    directions = [(-1,-1),(-1,0),(-1,1),(0,-1),
                  (0,1),(1,-1),(1,0),(1,1)]
    
    open_set = [(0 + heuristic(start, goal), 0, start)]
    g_score = {start: 0}
    parent = {}
    
    while open_set:
        f, g, current = heapq.heappop(open_set)
        
        if current == goal:
            # 回溯路径
            path = []
            while current in parent:
                path.append(current)
                current = parent[current]
            path.append(start)
            return path[::-1]
        
        for dr, dc in directions:
            neighbor = (current[0]+dr, current[1]+dc)
            if (0 <= neighbor[0] < rows and 
                0 <= neighbor[1] < cols and
                grid[neighbor[0]][neighbor[1]] == 0):
                
                # 对角线移动代价为 √2
                move_cost = 1.414 if dr != 0 and dc != 0 else 1.0
                new_g = g + move_cost
                
                if new_g < g_score.get(neighbor, float('inf')):
                    g_score[neighbor] = new_g
                    parent[neighbor] = current
                    f_new = new_g + heuristic(neighbor, goal)
                    heapq.heappush(open_set, (f_new, new_g, neighbor))
    
    return None  # 无可达路径
```

#### 14.2.3 A\* 的变体

| 变体 | 改进 | 使用场景 |
|------|------|---------|
| Weighted A\* | $f = g + \epsilon \cdot h$，$\epsilon > 1$ 加速搜索 | 需要快速次优解 |
| Theta\* | 允许非网格方向移动，路径更平滑 | 全向机器人 |
| D\* Lite | 动态地图变化时增量更新路径 | 未知或变化环境 |
| ARA\* | 从高 $\epsilon$ 快速求解，逐步降低至最优 | 时间受限的规划 |

#### 14.2.4 采样规划算法：RRT

**RRT（Rapidly-exploring Random Tree）** 通过随机采样快速探索高维空间，适合复杂环境和高自由度系统。

```
RRT(start, goal, map, max_iter):
  tree = {start}
  
  for i = 1 to max_iter:
    x_rand = random_sample(map)          # 随机采样
    x_near = nearest(tree, x_rand)       # 找最近节点
    x_new = steer(x_near, x_rand, step)  # 沿方向延伸
    
    if collision_free(x_near, x_new):     # 碰撞检测
      tree.add(x_new)
      tree.add_edge(x_near, x_new)
      
      if distance(x_new, goal) < threshold:
        return extract_path(tree, start, x_new)
  
  return failure
```

**RRT\*** 是 RRT 的改进版，增加了重连接操作（Rewiring），保证渐近最优。

| 算法 | 最优性 | 连通空间 | 高维适用 | 计算复杂度 |
|------|--------|---------|---------|-----------|
| A\* | 最优 | 离散网格 | 维度灾难 | $O(V \log V)$ |
| RRT | 概率完备 | 连续空间 | 适用 | $O(n \log n)$ |
| RRT\* | 渐近最优 | 连续空间 | 适用 | $O(n \log n)$ |

---

### 14.3 局部规划与避障

全局规划提供粗略的路径参考，但机器人需要在实际运动中处理未知障碍、动态环境和运动学约束。这是局部规划器的职责。

#### 14.3.1 DWA（Dynamic Window Approach）

DWA 是经典的局部避障算法，在速度空间中搜索最优控制指令。

**核心思想**：

1. **速度空间采样**：在允许的线速度 $(v)$ 和角速度 $(\omega)$ 范围内，考虑加速度约束，确定当前可达的速度窗口；
2. **轨迹仿真**：对窗口内的每组 $(v, \omega)$，仿真短期内（如 1-2 秒）机器人的运动轨迹；
3. **评价函数**：对每条轨迹打分，综合考虑：
   - **目标朝向**：轨迹终点与目标方向的对齐度；
   - **障碍距离**：轨迹上最近障碍物的距离；
   - **速度大小**：偏好较高速度（高效移动）；
4. **选择最优**：选择得分最高的 $(v, \omega)$ 作为当前控制指令。

```bob
┌──────────────────────────────────────────────────┐
│              DWA 工作流程                          │
│                                                  │
│  ┌────────────┐    ┌──────────────────┐          │
│  │ 当前速度   │───►│  动态窗口计算    │          │
│  │ "v, ω"    │    │  加速度约束      │          │
│  └────────────┘    └────────┬─────────┘          │
│                             │                    │
│                    ┌────────▼─────────┐          │
│                    │  速度空间采样    │          │
│                    │  N 组 "(v, ω)"  │          │
│                    └────────┬─────────┘          │
│                             │                    │
│                    ┌────────▼─────────┐          │
│  ┌──────────┐     │  轨迹仿真      │          │
│  │ 障碍物   │────►│  每组 → 轨迹弧 │          │
│  └──────────┘     └────────┬─────────┘          │
│                             │                    │
│  ┌──────────┐     ┌────────▼─────────┐          │
│  │ 目标点   │────►│  多目标评价函数  │          │
│  └──────────┘     └────────┬─────────┘          │
│                             │                    │
│                    ┌────────▼─────────┐          │
│                    │  最优 "(v, ω)"   │          │
│                    │  → "cmd_vel"     │          │
│                    └──────────────────┘          │
└──────────────────────────────────────────────────┘
```

#### 14.3.2 TEB（Timed Elastic Band）

TEB 将路径建模为带时间信息的弹性带，通过图优化同时考虑路径平滑性、时间最优和运动学约束。

与 DWA 对比：

| 特性 | DWA | TEB |
|------|-----|-----|
| 方法 | 速度空间采样 | 弹性带图优化 |
| 路径平滑性 | 一般 | 优秀 |
| 运动学约束 | 隐式（速度/加速度限制） | 显式（曲率、最小转弯半径） |
| 计算量 | 低 | 中 |
| 动态避障 | 即时响应 | 较好（可预测障碍运动） |
| 适用平台 | 差速、全向 | 差速、全向、阿克曼 |

#### 14.3.3 VFH（Vector Field Histogram）

VFH 将传感器数据投射为极坐标直方图，寻找安全通行的方向：

1. 将 360° 划分为 $n$ 个扇区（如每 5° 一个，共 72 个扇区）；
2. 累加每个扇区内障碍物的密度/距离权重；
3. 在直方图中寻找足够宽的低密度谷（valley）；
4. 从候选谷中选择最接近目标方向的进行通行。

---

### 14.4 代价地图（Costmap）

#### 14.4.1 什么是代价地图

代价地图是导航系统对环境的一种量化表示，将每个栅格赋予一个 0-255 的代价值：

| 代价值 | 含义 |
|--------|------|
| 0 | 空闲空间 |
| 1-252 | 不同程度的通行代价 |
| 253 | 膨胀区域（靠近障碍） |
| 254 | 致命障碍（已知障碍） |
| 255 | 未知区域 |

#### 14.4.2 代价地图层

Nav2 的代价地图由多个层叠加而成：

```bob
┌────────────────────────────────────────────┐
│            代价地图分层结构                  │
│                                            │
│  ┌───────────────────────────────────────┐  │
│  │          Master Costmap               │  │
│  │       （各层叠加的结果）              │  │
│  └───────────────────┬───────────────────┘  │
│                      │ 叠加                 │
│  ┌───────────┬───────┴──────┬────────────┐  │
│  │           │              │            │  │
│  ▼           ▼              ▼            ▼  │
│ ┌─────┐  ┌──────┐  ┌───────────┐ ┌─────┐  │
│ │静态层│  │障碍层│  │  膨胀层   │ │其他 │  │
│ │     │  │     │  │           │ │自定义│  │
│ │已知 │  │实时 │  │ 机器人    │ │ 层  │  │
│ │地图 │  │激光 │  │ 半径膨胀  │ │     │  │
│ └─────┘  └──────┘  └───────────┘ └─────┘  │
└────────────────────────────────────────────┘
```

- **静态层（Static Layer）**：来自预先构建的地图（SLAM 的输出），标记已知障碍和空闲区域；
- **障碍层（Obstacle Layer）**：根据激光雷达、深度相机等实时传感器数据，动态更新障碍物信息；
- **膨胀层（Inflation Layer）**：将障碍物向外膨胀，形成安全缓冲区。膨胀半径通常等于机器人半径 + 安全裕量；
- **自定义层**：可添加禁行区域、限速区域、虚拟墙等。

#### 14.4.3 全局代价地图 vs 局部代价地图

| 特性 | 全局代价地图 | 局部代价地图 |
|------|------------|------------|
| 范围 | 整个地图 | 机器人周围窗口（如 5×5m） |
| 更新频率 | 低（静态为主） | 高（传感器实时更新） |
| 用途 | 全局规划器使用 | 局部规划器使用 |
| 滚动 | 固定 | 跟随机器人移动 |

---

### 14.5 定位：AMCL

当使用已知地图导航时，机器人需要在地图上定位自身。AMCL（Adaptive Monte Carlo Localization）是 ROS2 中最常用的定位方法。

#### 14.5.1 粒子滤波定位

AMCL 基于粒子滤波（蒙特卡洛定位）：

1. **初始化**：在地图上均匀撒布 $N$ 个粒子（每个粒子代表一种可能的位姿）；
2. **预测**：根据里程计数据，按运动模型移动所有粒子（加入噪声）；
3. **更新**：将激光扫描与地图对比，计算每个粒子的权重（观测似然）；
4. **重采样**：按权重重新采样粒子——权重高的粒子被复制，低的被淘汰；
5. **收敛**：经过多次迭代，粒子聚集到真实位姿附近。

```bob
┌────────────────────────────────────────────────────┐
│           AMCL 粒子滤波定位流程                     │
│                                                    │
│  步骤1: 初始化                步骤2: 预测           │
│  ┌──────────────┐            ┌──────────────┐      │
│  │ . . . . . .  │            │ . . . . . .  │      │
│  │ . . . . . .  │  里程计    │  . . . . . . │      │
│  │ . . . . . .  │ ────────►  │   . . . . .  │      │
│  │ . . . . . .  │  +噪声     │    . . . . . │      │
│  └──────────────┘            └──────────────┘      │
│                                                    │
│  步骤3: 更新权重              步骤4: 重采样         │
│  ┌──────────────┐            ┌──────────────┐      │
│  │ . . o . . .  │  激光匹配  │       O      │      │
│  │  . . o . .   │ ────────►  │      OOO     │      │
│  │   . o O o .  │  权重大=O  │       O      │      │
│  │    . . . . . │            │              │      │
│  └──────────────┘            └──────────────┘      │
│                              粒子聚集=定位成功      │
└────────────────────────────────────────────────────┘
```

#### 14.5.2 AMCL 配置要点

```yaml
amcl:
  ros__parameters:
    # 粒子数量
    min_particles: 500
    max_particles: 2000
    
    # 运动模型参数（差速驱动）
    robot_model_type: differential
    alpha1: 0.2  # 旋转-旋转噪声
    alpha2: 0.2  # 旋转-平移噪声
    alpha3: 0.2  # 平移-平移噪声
    alpha4: 0.2  # 平移-旋转噪声
    
    # 激光模型
    laser_model_type: likelihood_field
    laser_max_range: 12.0
    max_beams: 60
    z_hit: 0.5
    z_rand: 0.5
    sigma_hit: 0.2
    
    # 更新阈值
    update_min_d: 0.25   # 最小平移 0.25m 触发更新
    update_min_a: 0.2    # 最小旋转 0.2rad 触发更新
```

---

### 14.6 ROS2 Nav2 导航栈

#### 14.6.1 Nav2 架构概览

Nav2 是 ROS2 的标准导航框架，采用行为树（Behavior Tree）驱动的模块化架构：

```bob
┌──────────────────────────────────────────────────────────────┐
│                     Nav2 导航栈架构                           │
│                                                              │
│  ┌────────────────────────────────────────────────────────┐  │
│  │              行为树（BT Navigator）                     │  │
│  │  NavigateToPose → ComputePath → FollowPath → Recovery  │  │
│  └─────┬────────────────┬──────────────┬──────────────────┘  │
│        │                │              │                     │
│  ┌─────▼─────┐  ┌──────▼──────┐  ┌───▼─────────┐           │
│  │ 全局规划  │  │  局部控制   │  │  恢复行为   │           │
│  │ Planner   │  │ Controller  │  │ Recovery    │           │
│  │ Server    │  │ Server      │  │ Server      │           │
│  └─────┬─────┘  └──────┬──────┘  └─────────────┘           │
│        │                │                                    │
│  ┌─────▼────────────────▼─────────────────────────────┐     │
│  │                代价地图                             │     │
│  │  全局 Costmap          局部 Costmap                │     │
│  └────────────────────────────────────────────────────┘     │
│        ▲                ▲                                    │
│  ┌─────┴─────┐  ┌──────┴──────┐                             │
│  │  地图服务 │  │    AMCL     │                             │
│  │ Map Server│  │    定位     │                             │
│  └───────────┘  └─────────────┘                             │
└──────────────────────────────────────────────────────────────┘
```

核心组件：

- **BT Navigator**：行为树导航器，协调规划、控制和恢复行为的执行顺序；
- **Planner Server**：全局路径规划服务，默认使用 NavFn（基于 Dijkstra/A\*）；
- **Controller Server**：局部控制服务，默认使用 DWB（DWA 的改进版）；
- **Recovery Server**：恢复行为（如旋转、后退、清除代价地图），在导航失败时尝试恢复；
- **Costmap**：代价地图（全局+局部），管理导航的环境感知。

#### 14.6.2 Nav2 安装与基本配置

```bash
# 安装 Nav2 全套
sudo apt install ros-humble-navigation2 ros-humble-nav2-bringup

# 安装 Turtlebot3 仿真（用于学习）
sudo apt install ros-humble-turtlebot3-gazebo ros-humble-turtlebot3-navigation2
export TURTLEBOT3_MODEL=burger
```

#### 14.6.3 Nav2 参数配置

Nav2 的配置文件是一个大型 YAML 文件，以下是核心部分：

```yaml
# nav2_params.yaml
bt_navigator:
  ros__parameters:
    default_bt_xml_filename: "navigate_w_replanning_and_recovery.xml"
    plugin_lib_names:
      - nav2_compute_path_to_pose_action_bt_node
      - nav2_follow_path_action_bt_node
      - nav2_back_up_action_bt_node
      - nav2_spin_action_bt_node
      - nav2_wait_action_bt_node

controller_server:
  ros__parameters:
    controller_frequency: 20.0
    FollowPath:
      plugin: "dwb_core::DWBLocalPlanner"
      min_vel_x: 0.0
      max_vel_x: 0.26
      max_vel_theta: 1.0
      min_speed_xy: 0.0
      max_speed_xy: 0.26
      acc_lim_x: 2.5
      acc_lim_theta: 3.2
      decel_lim_x: -2.5
      decel_lim_theta: -3.2

planner_server:
  ros__parameters:
    GridBased:
      plugin: "nav2_navfn_planner/NavfnPlanner"
      tolerance: 0.5
      use_astar: true
      allow_unknown: true

global_costmap:
  global_costmap:
    ros__parameters:
      update_frequency: 1.0
      publish_frequency: 1.0
      robot_base_frame: base_link
      global_frame: map
      resolution: 0.05
      plugins: ["static_layer", "obstacle_layer", "inflation_layer"]
      inflation_layer:
        inflation_radius: 0.55
        cost_scaling_factor: 2.58
      obstacle_layer:
        observation_sources: scan
        scan:
          topic: /scan
          max_obstacle_height: 2.0
          clearing: true
          marking: true

local_costmap:
  local_costmap:
    ros__parameters:
      update_frequency: 5.0
      publish_frequency: 2.0
      rolling_window: true
      width: 3
      height: 3
      resolution: 0.05
      plugins: ["voxel_layer", "inflation_layer"]
```

#### 14.6.4 运行完整导航示例

```bash
# 终端1：启动 Turtlebot3 仿真
export TURTLEBOT3_MODEL=burger
ros2 launch turtlebot3_gazebo turtlebot3_world.launch.py

# 终端2：启动导航
ros2 launch turtlebot3_navigation2 navigation2.launch.py \
  use_sim_time:=true map:=$HOME/map.yaml

# 在 RViz2 中：
# 1. 使用 "2D Pose Estimate" 设置初始位置
# 2. 使用 "Nav2 Goal" 设置目标位置
# 3. 观察全局路径（绿线）和局部轨迹（红线）
```

---

### 14.7 行为树（Behavior Tree）

#### 14.7.1 为什么用行为树

Nav2 使用行为树（BT）替代传统的有限状态机（FSM）来管理导航逻辑。行为树的优势：

- **模块化**：每个行为是独立节点，易于组合和复用；
- **可扩展**：添加新行为只需创建新节点，无需修改现有逻辑；
- **可读性**：树结构直观表达任务优先级和执行顺序；
- **可调试**：通过 BT 可视化工具实时监控每个节点的状态。

#### 14.7.2 行为树基本结构

```bob
┌──────────────────────────────────────────────────────┐
│           Nav2 默认行为树（简化）                      │
│                                                      │
│                  ┌───────────┐                        │
│                  │ Sequence  │                        │
│                  └─────┬─────┘                        │
│               ┌────────┼────────────┐                │
│               │        │            │                │
│         ┌─────▼─────┐ ┌▼──────┐ ┌──▼────────────┐   │
│         │ Compute   │ │Follow │ │ Recovery       │   │
│         │ PathTo    │ │ Path  │ │ Fallback       │   │
│         │ Pose      │ │       │ │                │   │
│         └───────────┘ └───────┘ │ ┌────┐ ┌────┐ │   │
│                                 │ │Spin│ │Back│ │   │
│                                 │ └────┘ └────┘ │   │
│                                 └────────────────┘   │
└──────────────────────────────────────────────────────┘
```

核心节点类型：

| 节点类型 | 行为 | 示例 |
|---------|------|------|
| Sequence | 依次执行子节点，全部成功才成功 | ComputePath → FollowPath |
| Fallback | 依次尝试子节点，任一成功就成功 | Recovery 策略选择 |
| Action | 执行具体操作 | ComputePathToPose, FollowPath |
| Condition | 检查条件 | IsGoalReached, IsBatteryLow |
| Decorator | 修饰子节点行为 | RateController, RecoveryNode |

---

### 14.8 导航实战：自定义导航系统

#### 14.8.1 自定义全局规划器

在 Nav2 中，可以通过插件机制实现自定义规划器：

```cpp
// my_planner.hpp
#include "nav2_core/global_planner.hpp"

class MyPlanner : public nav2_core::GlobalPlanner {
public:
    void configure(
        const rclcpp_lifecycle::LifecycleNode::WeakPtr & parent,
        std::string name,
        std::shared_ptr<tf2_ros::Buffer> tf,
        std::shared_ptr<nav2_costmap_2d::Costmap2DROS> costmap_ros) override;
    
    void cleanup() override;
    void activate() override;
    void deactivate() override;
    
    nav_msgs::msg::Path createPlan(
        const geometry_msgs::msg::PoseStamped & start,
        const geometry_msgs::msg::PoseStamped & goal) override;
};
```

#### 14.8.2 多点巡航（Waypoint Following）

```python
# waypoint_follower.py
import rclpy
from rclpy.node import Node
from geometry_msgs.msg import PoseStamped
from nav2_simple_commander.robot_navigator import BasicNavigator

class WaypointFollower(Node):
    def __init__(self):
        super().__init__('waypoint_follower')
        self.navigator = BasicNavigator()
        
        # 等待 Nav2 激活
        self.navigator.waitUntilNav2Active()
        
        # 定义巡航点
        waypoints = [
            self.create_pose(1.0, 0.0, 0.0),
            self.create_pose(2.0, 1.0, 1.57),
            self.create_pose(0.0, 2.0, 3.14),
            self.create_pose(0.0, 0.0, 0.0),
        ]
        
        # 开始巡航
        self.navigator.followWaypoints(waypoints)
        
        while not self.navigator.isTaskComplete():
            feedback = self.navigator.getFeedback()
            self.get_logger().info(
                f'Executing waypoint {feedback.current_waypoint + 1}/'
                f'{len(waypoints)}')
        
        result = self.navigator.getResult()
        self.get_logger().info(f'Navigation result: {result}')
    
    def create_pose(self, x, y, yaw):
        pose = PoseStamped()
        pose.header.frame_id = 'map'
        pose.pose.position.x = x
        pose.pose.position.y = y
        pose.pose.orientation.z = math.sin(yaw / 2)
        pose.pose.orientation.w = math.cos(yaw / 2)
        return pose
```

#### 14.8.3 与 micro-ROS 底盘集成

结合第 13 章的 micro-ROS 底盘节点，完整的导航数据流：

```bob
┌─────────────────────────────────────────────────────────────┐
│           Nav2 + micro-ROS 导航数据流                        │
│                                                             │
│  ┌─────────────────────────────────────────────────────┐    │
│  │                    Nav2                             │    │
│  │  目标点 → 全局规划 → 局部规划 → "cmd_vel"          │    │
│  └──────────────────┬──────────────▲───────────────────┘    │
│                     │ "cmd_vel"    │ /odom, /scan           │
│  ┌──────────────────▼──────────────┴───────────────────┐    │
│  │              micro-ROS Agent                        │    │
│  └──────────────────┬──────────────▲───────────────────┘    │
│                     │ UART         │ UART                   │
│  ┌──────────────────▼──────────────┴───────────────────┐    │
│  │            STM32  底盘控制器                         │    │
│  │  订阅 "cmd_vel" → 差速运动学 → PID → 电机 PWM     │    │
│  │  编码器 → 里程计 → 发布 /odom                       │    │
│  │  激光雷达 → 转发 /scan                              │    │
│  └─────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
```

---

### 14.9 本章小结与拓展资源

#### 14.9.1 关键知识点回顾

1. **全局规划**：A\* 算法（启发搜索、最优路径）、RRT（采样规划、高维空间）；
2. **局部规划**：DWA（速度空间搜索）、TEB（弹性带优化）、VFH（直方图避障）；
3. **代价地图**：静态层+障碍层+膨胀层的分层架构，全局/局部代价地图分工；
4. **定位**：AMCL 粒子滤波在已知地图上的定位方法；
5. **Nav2 架构**：行为树驱动的模块化导航栈，Planner/Controller/Recovery Server 协作；
6. **工程实践**：Nav2 配置、多点巡航、与 micro-ROS 底盘的集成。

#### 14.9.2 推荐学习资源

| 资源 | 说明 |
|------|------|
| [Nav2 官方文档](https://navigation.ros.org/) | 完整 API、教程和配置指南 |
| [Nav2 Simple Commander](https://github.com/ros-planning/navigation2/tree/main/nav2_simple_commander) | Python API 快速上手 |
| 《Planning Algorithms》(LaValle) | 路径规划理论经典 |
| 《Probabilistic Robotics》Ch. 5-7 | 粒子滤波、栅格地图、定位 |
| [MoveBase Flex](http://wiki.ros.org/move_base_flex) | ROS1 导航框架参考 |

#### 14.9.3 课后练习

1. **算法实现**：用 Python 实现 A\* 算法，在 50×50 的栅格地图上（含随机障碍物）搜索最短路径，可视化搜索过程和最终路径。
2. **算法对比**：对比 Dijkstra 和 A\*（使用不同启发函数）在同一地图上的搜索效率（扩展节点数、路径长度、计算时间）。
3. **Nav2 配置**：修改 Nav2 的参数，将全局规划器从 NavFn 切换为 Smac Planner（Hybrid A\*），观察路径平滑度的变化。
4. **多点巡航**：编写一个 ROS2 节点，使用 Nav2 Simple Commander 实现机器人在 5 个巡航点之间循环导航。
5. **综合挑战**：结合 SLAM（第 14 章）和 Nav2，在 Gazebo 仿真中实现机器人自主探索未知环境——边建图边导航到未探索区域。

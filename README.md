# TurtleBot4 Autonomous Priority Navigation

Sistema de navegación autónoma con gestión de prioridades para TurtleBot4, desarrollado mediante ROS 2 Humble, Navigation2 (Nav2) y Behavior Trees personalizados.

**Autora:** Raysha Enciso Ravelo

## 1. Descripción

El proyecto implementa un sistema de navegación capaz de interrumpir temporalmente una misión normal para atender una solicitud urgente.

Una vez alcanzado el objetivo prioritario, el sistema permite retomar la navegación hacia la meta original.

## 2. Arquitectura

El sistema utiliza:

- ROS 2 Humble
- TurtleBot4
- Navigation2 (Nav2)
- BehaviorTree.CPP v3
- Gazebo
- RViz2
- Plugins personalizados en C++

### Componentes principales

| Componente | Función |
|---|---|
| NavigationGoalSelector | Selecciona la meta normal o urgente |
| UrgentGoalCompleted | Registra la finalización de una emergencia |
| ComputePathToPose | Calcula la trayectoria global |
| FollowPath | Ejecuta el seguimiento de trayectoria |
| RecoveryNode | Gestiona los reintentos del árbol |

El árbol actual utiliza `AlwaysSuccess` como nodo auxiliar de reintento. No implementa todavía una estrategia completa de recuperación ante fallos.

## 3. Funcionamiento

Secuencia de navegación:

1. El robot recibe una meta normal A.
2. Nav2 comienza a ejecutar la trayectoria.
3. Se publica una meta urgente B en `/urgent_goal`.
4. El sistema prioriza B sobre A.
5. El robot navega hacia B.
6. Al completar la emergencia, recupera la meta A.
7. Continúa la navegación normal.

## 4. Estructura del proyecto

```text
tb4_autonomy/
├── behavior_trees/
│   └── urgent_route.xml
├── config/
│   └── nav2_urgent.yaml
├── launch/
│   └── urgent_navigation.launch.py
├── maps/
│   ├── mapa_laboratorio.yaml
│   └── mapa_laboratorio.pgm
├── src/
├── CMakeLists.txt
├── package.xml
└── README.md
```

## 5. Requisitos

- Ubuntu 22.04
- ROS 2 Humble
- Navigation2
- TurtleBot4 Navigation
- BehaviorTree.CPP v3
- Colcon
- Entorno de simulación TurtleBot4 en Gazebo

Las dependencias ROS 2 deben estar instaladas antes de compilar.

## 6. Compilación

Clonar el repositorio dentro de un workspace ROS 2:

```bash
mkdir -p ~/tb4_ws/src
cd ~/tb4_ws/src

git clone https://github.com/rayshaenciso-crypto/tb4_autonomy.git

cd ~/tb4_ws

source /opt/ros/humble/setup.bash

colcon build --packages-select tb4_autonomy

source install/setup.bash
```

## 7. Ejecución

Con la simulación TurtleBot4 y sus interfaces de navegación preparadas, ejecutar:

```bash
source /opt/ros/humble/setup.bash
source ~/tb4_ws/install/setup.bash

ros2 launch tb4_autonomy urgent_navigation.launch.py use_sim_time:=true
```

El lanzamiento inicia los componentes Nav2 utilizando la configuración personalizada del proyecto.

La simulación de Gazebo debe iniciarse por separado.

## 8. Publicación de una emergencia

Ejemplo:

```bash
ros2 topic pub --once /urgent_goal geometry_msgs/msg/PoseStamped \
"{header: {frame_id: map}, pose: {position: {x: -9.73, y: -2.15, z: 0.0}, orientation: {w: 1.0}}}"
```

Las coordenadas deben adaptarse al mapa utilizado.

## 9. Pruebas realizadas

Durante las pruebas en simulación se observó:

- Navegación hacia una meta normal.
- Recepción de una emergencia durante el desplazamiento.
- Cambio de trayectoria hacia la meta urgente.
- Llegada a la coordenada prioritaria.
- Reanudación de la navegación hacia el objetivo original.

## 10. Limitaciones y trabajo futuro

- Incorporar estrategias completas de recuperación Nav2.
- Validar sistemáticamente la evasión de obstáculos dinámicos.
- Realizar pruebas de repetibilidad y registrar métricas.
- Validar el sistema en un TurtleBot4 físico.

## 11. Autoría

Desarrollado por Raysha Enciso Ravelo.

Proyecto de navegación autónoma mediante ROS 2, Nav2 y Behavior Trees personalizados.

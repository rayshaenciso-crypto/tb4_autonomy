cmake_minimum_required(VERSION 3.8)
project(tb4_autonomy)

# ==========================================================
# CONFIGURACION DEL COMPILADOR
# ==========================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ==========================================================
# DEPENDENCIAS
# ==========================================================

find_package(ament_cmake REQUIRED)

find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)

find_package(nav2_msgs REQUIRED)
find_package(nav2_behavior_tree REQUIRED)

find_package(behaviortree_cpp_v3 REQUIRED)
find_package(geometry_msgs REQUIRED)

# ==========================================================
# NODO ROS 2 INDEPENDIENTE: LOG STATUS
# ==========================================================

add_executable(log_status_node
  src/log_status_node.cpp
)

ament_target_dependencies(log_status_node
  rclcpp
)

# ==========================================================
# PLUGIN 1: DETECCION DE META URGENTE
# ==========================================================

add_library(urgent_goal_bt_node SHARED
  src/urgent_goal_node.cpp
)

ament_target_dependencies(urgent_goal_bt_node
  rclcpp
  rclcpp_lifecycle
  nav2_behavior_tree
  behaviortree_cpp_v3
  geometry_msgs
)

# ==========================================================
# PLUGIN 2: OBTENER META URGENTE
# ==========================================================

add_library(get_urgent_goal_bt_node SHARED
  src/get_urgent_goal_node.cpp
)

ament_target_dependencies(get_urgent_goal_bt_node
  rclcpp
  nav2_behavior_tree
  behaviortree_cpp_v3
  geometry_msgs
)

# ==========================================================
# PLUGIN 3: LIMPIAR META URGENTE
# ==========================================================

add_library(clear_urgent_goal_bt_node SHARED
  src/clear_urgent_goal_node.cpp
)

ament_target_dependencies(clear_urgent_goal_bt_node
  rclcpp
  nav2_behavior_tree
  behaviortree_cpp_v3
  geometry_msgs
)

# ==========================================================
# PLUGIN 4: SUPERVISOR DE PRIORIDADES BT
# ==========================================================

add_library(navigation_goal_selector_bt_node SHARED
  src/navigation_goal_selector.cpp
)

ament_target_dependencies(navigation_goal_selector_bt_node
  rclcpp
  rclcpp_lifecycle
  nav2_behavior_tree
  behaviortree_cpp_v3
  geometry_msgs
)

# ==========================================================
# PLUGIN 5: CONFIRMACION DE EMERGENCIA COMPLETADA
# ==========================================================

add_library(urgent_goal_completed_bt_node SHARED
  src/urgent_goal_completed_node.cpp
)

ament_target_dependencies(urgent_goal_completed_bt_node
  behaviortree_cpp_v3
)

# ==========================================================
# NODO 6: SUPERVISOR DE NAVEGACION POR PRIORIDADES
# ==========================================================

add_executable(priority_navigation_node
  src/priority_navigation_node.cpp
)

ament_target_dependencies(priority_navigation_node
  rclcpp
  rclcpp_action
  nav2_msgs
  geometry_msgs
)

# ==========================================================
# INSTALACION DE EJECUTABLES
# ==========================================================

install(TARGETS
  log_status_node
  priority_navigation_node
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

# ==========================================================
# INSTALACION DE LOS CINCO PLUGINS BT
# ==========================================================

install(TARGETS
  urgent_goal_bt_node
  get_urgent_goal_bt_node
  clear_urgent_goal_bt_node
  navigation_goal_selector_bt_node
  urgent_goal_completed_bt_node
  LIBRARY DESTINATION lib
)

# ==========================================================
# INSTALACION DE LOS ARBOLES DE COMPORTAMIENTO
# ==========================================================

install(DIRECTORY
  behavior_trees
  DESTINATION share/${PROJECT_NAME}
)

# ==========================================================
# REGISTRO DEL PAQUETE ROS 2
# ==========================================================


# ==========================================================
# INSTALACION DE CONFIGURACIONES Y MAPAS
# ==========================================================

install(DIRECTORY
  config
  maps
  DESTINATION share/${PROJECT_NAME}
)


# ==========================================================
# INSTALACION DE ARCHIVOS DE LANZAMIENTO
# ==========================================================

install(DIRECTORY
  launch
  DESTINATION share/${PROJECT_NAME}
)

# Exportacion de plugins personalizados para BehaviorTree.CPP v3
foreach(bt_plugin
  urgent_goal_bt_node
  get_urgent_goal_bt_node
  clear_urgent_goal_bt_node
  navigation_goal_selector_bt_node
  urgent_goal_completed_bt_node
)
  target_compile_definitions(${bt_plugin} PRIVATE BT_PLUGIN_EXPORT)
endforeach()

ament_package()

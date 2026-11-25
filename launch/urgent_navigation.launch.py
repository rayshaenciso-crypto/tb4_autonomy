import os
import tempfile

from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration

from launch_ros.parameter_descriptions import ParameterFile


def generate_launch_description():

    # Localizar paquetes instalados
    autonomy_share = get_package_share_directory('tb4_autonomy')
    navigation_share = get_package_share_directory('turtlebot4_navigation')

    # Localizar Behavior Tree
    bt_xml = os.path.join(
        autonomy_share,
        'behavior_trees',
        'urgent_route.xml'
    )

    # Localizar configuracion Nav2
    nav2_yaml = os.path.join(
        autonomy_share,
        'config',
        'nav2_urgent.yaml'
    )

    # Leer configuracion original
    with open(nav2_yaml, 'r') as file:
        contenido = file.read()

    # Sustituir ruta temporal por ruta instalada
    # Resolver automaticamente la ruta del Behavior Tree
    marcador = "__TB4_BT_XML__"

    if marcador not in contenido:
        raise RuntimeError(
            "No se encontro el marcador del Behavior Tree"
        )

    contenido = contenido.replace(marcador, bt_xml)

    # Crear configuracion temporal
    with tempfile.NamedTemporaryFile(
        mode='w',
        suffix='.yaml',
        prefix='tb4_nav2_',
        delete=False
    ) as archivo:
        archivo.write(contenido)
        yaml_temporal = archivo.name

    # Lanzamiento original TurtleBot4
    nav2_launch = os.path.join(
        navigation_share,
        'launch',
        'nav2.launch.py'
    )

    return LaunchDescription([
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(nav2_launch),
            launch_arguments={
                'use_sim_time': LaunchConfiguration(
                    'use_sim_time',
                    default='true'
                ),
                'params_file': yaml_temporal,
                'namespace': ''
            }.items()
        )
    ])

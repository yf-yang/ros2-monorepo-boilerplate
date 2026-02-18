from setuptools import find_packages, setup

package_name = "demo_py_node"

setup(
    name=package_name,
    version="0.1.0",
    packages=find_packages(exclude=["test"]),
    data_files=[
        (
            "share/ament_index/resource_index/packages",
            [f"resource/{package_name}"],
        ),
        (f"share/{package_name}", ["package.xml"]),
    ],
    install_requires=["setuptools"],
    zip_safe=True,
    maintainer="Dev Team",
    maintainer_email="dev@example.com",
    description="Demo Python ROS 2 node for bidirectional topic messaging.",
    license="Apache-2.0",
    entry_points={
        "console_scripts": [
            "demo_py_node = demo_py_node.demo_py_node:main",
        ],
    },
)

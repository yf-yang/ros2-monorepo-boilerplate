import unittest

from demo_py_node.demo_py_node import DemoPyNode


class TestImportDemoPyNode(unittest.TestCase):
    def test_class_name(self) -> None:
        self.assertEqual(DemoPyNode.__name__, "DemoPyNode")


if __name__ == "__main__":
    unittest.main()

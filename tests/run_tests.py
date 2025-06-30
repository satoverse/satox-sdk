import unittest
import sys
import os
import subprocess

def run_tests():
    # Get the directory containing this script
    test_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.abspath(os.path.join(test_dir, '..'))
    build_dir = os.path.join(project_root, 'build')
    
    # Set PYTHONPATH to project root for all Python test runs
    env = os.environ.copy()
    env['PYTHONPATH'] = project_root + (os.pathsep + env.get('PYTHONPATH', ''))

    # 1. Run Python unittest tests
    print("\n=== Running Python unittest tests ===")
    loader = unittest.TestLoader()
    suite = loader.discover(test_dir, pattern='test_*.py')
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    if not result.wasSuccessful():
        print("Python unittest tests failed.")
        return 1

    # 2. Run pytest for test_suite.py
    print("\n=== Running pytest (test_suite.py) ===")
    pytest_result = subprocess.run([sys.executable, '-m', 'pytest', os.path.join(test_dir, 'test_suite.py')], env=env)
    if pytest_result.returncode != 0:
        print("pytest tests failed.")
        return pytest_result.returncode

    # 3. Run Go tests
    print("\n=== Running Go tests ===")
    go_result = subprocess.run(['go', 'test', './...'], cwd=project_root)
    if go_result.returncode != 0:
        print("Go tests failed.")
        return go_result.returncode

    # 4. Run C++ tests via CTest
    print("\n=== Running C++ tests (CTest) ===")
    ctest_result = subprocess.run(['ctest', '--output-on-failure'], cwd=build_dir)
    if ctest_result.returncode != 0:
        print("C++ tests failed.")
        return ctest_result.returncode

    print("\nAll tests passed!")
    return 0

if __name__ == '__main__':
    sys.exit(run_tests()) 
# Satox Core SDK - Root Test File

# This file enables testing for the project.
# Individual tests are defined within the CMakeLists.txt of each component
# in the src/ directory (e.g., src/core/tests/CMakeLists.txt).

enable_testing()

# The root CMakeLists.txt will add the component directories,
# which will in turn add their own tests if testing is enabled.

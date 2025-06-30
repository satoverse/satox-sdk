"""Pytest configuration for the Satox SDK Python bindings tests."""

import pytest

# Set asyncio as the default event loop for tests
pytest.register_assert_rewrite('pytest_asyncio') 
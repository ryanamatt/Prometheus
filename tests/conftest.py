import pytest

def pytest_addoption(parser):
    parser.addoption(
        "--update", action="store_true", default=False, help="Update .expected files with actual output"
    )

@pytest.fixture
def update_mode(request):
    return request.config.getoption("--update")
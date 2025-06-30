# Contributing to Satox SDK

Thank you for your interest in contributing to the Satox SDK project! This document provides guidelines and instructions for contributing to the project.

## Table of Contents
1. [Code of Conduct](#code-of-conduct)
2. [Getting Started](#getting-started)
3. [Development Workflow](#development-workflow)
4. [Code Style](#code-style)
5. [Testing](#testing)
6. [Documentation](#documentation)
7. [Pull Request Process](#pull-request-process)
8. [Release Process](#release-process)

## Code of Conduct

By participating in this project, you agree to abide by our Code of Conduct. Please read it before contributing.

### Our Pledge
We are committed to making participation in this project a harassment-free experience for everyone, regardless of level of experience, gender, gender identity and expression, sexual orientation, disability, personal appearance, body size, race, ethnicity, age, religion, or nationality.

### Our Standards
- Using welcoming and inclusive language
- Being respectful of differing viewpoints and experiences
- Gracefully accepting constructive criticism
- Focusing on what is best for the community
- Showing empathy towards other community members

## Getting Started

1. Fork the repository
2. Clone your fork:
   ```bash
   git clone https://github.com/your-username/satox-sdk.git
   ```
3. Create a new branch:
   ```bash
   git checkout -b feature/your-feature-name
   ```

## Development Workflow

1. **Setup Development Environment**
   ```bash
   # Install dependencies
   pip install -r requirements.txt
   
   # Setup pre-commit hooks
   pre-commit install
   ```

2. **Make Changes**
   - Follow the code style guidelines
   - Write tests for new features
   - Update documentation

3. **Test Your Changes**
   ```bash
   # Run all tests
   pytest
   
   # Run specific test file
   pytest tests/test_file.py
   ```

4. **Commit Your Changes**
   ```bash
   git add .
   git commit -m "feat: add new feature"
   ```

## Code Style

### Python
- Follow PEP 8 guidelines
- Use type hints
- Maximum line length: 88 characters
- Use Black for formatting
- Use isort for import sorting

### JavaScript/TypeScript
- Use ESLint and Prettier
- Follow Airbnb style guide
- Use TypeScript for type safety

### Go
- Follow Go standard formatting
- Use gofmt
- Follow Go best practices

## Testing

### Test Categories
1. **Unit Tests**
   - Test individual components
   - Mock external dependencies
   - Aim for high coverage

2. **Integration Tests**
   - Test component interactions
   - Use test fixtures
   - Test error cases

3. **Performance Tests**
   - Benchmark critical paths
   - Compare against baselines
   - Document results

### Running Tests
```bash
# Run all tests
pytest

# Run with coverage
pytest --cov=.

# Run specific test category
pytest tests/unit/
pytest tests/integration/
pytest tests/performance/
```

## Documentation

### Documentation Types
1. **Code Documentation**
   - Docstrings for all public APIs
   - Type hints and comments
   - Examples in docstrings

2. **User Documentation**
   - Installation guides
   - Usage examples
   - API reference
   - Troubleshooting guides

3. **Developer Documentation**
   - Architecture overview
   - Development setup
   - Contributing guidelines

### Documentation Standards
- Use Markdown for all documentation
- Include code examples
- Keep documentation up to date
- Use clear and concise language

## Pull Request Process

1. **Before Submitting**
   - Update documentation
   - Add tests
   - Run all tests
   - Check code style

2. **Pull Request Template**
   - Description of changes
   - Related issues
   - Type of change
   - Testing performed
   - Documentation updates

3. **Review Process**
   - Address review comments
   - Keep PR focused
   - Update as needed

## Release Process

1. **Version Bumping**
   - Follow semantic versioning
   - Update version in all relevant files
   - Update changelog

2. **Release Checklist**
   - All tests passing
   - Documentation updated
   - Changelog updated
   - Version bumped
   - Release notes prepared

3. **Publishing**
   - Create release tag
   - Build packages
   - Publish to package managers
   - Announce release

## Questions and Support

- GitHub Issues for bug reports
- GitHub Discussions for questions
- Discord for community support

Thank you for contributing to Satox SDK! 
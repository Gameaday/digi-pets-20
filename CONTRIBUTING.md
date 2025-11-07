# Contributing to DigiPets

Thank you for your interest in contributing to DigiPets! This document provides guidelines for contributing to the project.

## How to Contribute

1. **Fork the repository**
2. **Create a feature branch** (`git checkout -b feature/amazing-feature`)
3. **Make your changes**
4. **Add tests** for new functionality
5. **Ensure all tests pass** (`ctest`)
6. **Commit your changes** (`git commit -m 'Add amazing feature'`)
7. **Push to the branch** (`git push origin feature/amazing-feature`)
8. **Open a Pull Request**

## Development Setup

### Prerequisites
- C++20 compatible compiler (GCC 10+, Clang 12+, MSVC 2019+)
- CMake 3.20+
- Git

### Building

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
```

### Running Tests

```bash
cd build
ctest --output-on-failure
```

## Code Style

- Use modern C++20 features where appropriate
- Follow the existing code style
- Use meaningful variable and function names
- Add comments for complex logic
- Keep functions focused and small

## Testing

- Write unit tests for new functionality
- Ensure existing tests still pass
- Test edge cases
- Integration tests for API endpoints

## Pull Request Guidelines

- Keep PRs focused on a single feature or fix
- Update documentation if needed
- Add tests for new features
- Ensure CI/CD pipeline passes
- Reference any related issues

## Reporting Bugs

Use GitHub Issues and include:
- Description of the bug
- Steps to reproduce
- Expected behavior
- Actual behavior
- System information (OS, compiler version, etc.)

## Feature Requests

We welcome feature requests! Please:
- Check if the feature already exists
- Describe the feature clearly
- Explain the use case
- Consider implementation approach

## Code of Conduct

- Be respectful and inclusive
- Welcome newcomers
- Focus on constructive feedback
- Help maintain a positive community

## License

By contributing, you agree that your contributions will be licensed under the MIT License.

## Questions?

Feel free to open an issue for any questions or concerns!

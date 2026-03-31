# Implementation Summary for Problem 1939 - Plagiarism Detection

## Repository
- **URL**: https://github.com/ojbench/oj-eval-claude-code-025-20260401024531
- **Latest Commit**: 82bd4d1
- **Status**: Ready for evaluation

## Implementation Overview

### Files Created

1. **eval.h** - Shared header with S-expression parser
   - SExpr structure for representing programs
   - Parser class for parsing S-expressions
   - Helper functions for I/O and keyword detection

2. **main.cpp** - Combined executable (produces `code` binary)
   - Supports both "cheat" and "anticheat" modes via command-line argument
   - Integrates both transformer and similarity checker

3. **cheat.cpp** - Plagiarism transformer (standalone version)
   - Systematically renames variables and functions
   - Preserves program semantics
   - Generates functionally equivalent but structurally different code

4. **anticheat.cpp** - Plagiarism detector (standalone version)
   - Computes similarity between two programs (0-1 scale)
   - Uses multiple metrics: structural, feature-based, pattern-based
   - Handles "endprogram" separators correctly

5. **Build System**
   - **Makefile**: Builds `code` executable directly
   - **CMakeLists.txt**: Alternative cmake-based build
   - Both produce the required `code` executable

6. **.gitignore** - Prevents build artifacts from being committed

## Algorithm Details

### Cheat (Plagiarism Transformer)
- **Variable Renaming**: Maps original variable names to generated names (var_1, var_2, etc.)
- **Function Renaming**: Renames functions and their parameters (func_1, param_1, etc.)
- **Scope Handling**: Properly handles local scopes within functions
- **Builtin Preservation**: Keeps language keywords unchanged (function, block, set, if, while, array operations, etc.)

### Anticheat (Plagiarism Detector)
The similarity score is computed as a weighted combination of:

1. **Structural Similarity (40%)**: Tree-based comparison
   - Compares AST structure recursively
   - Treats renamed identifiers as similar (0.8 score)
   - Distinguishes builtins (must match exactly) from user-defined names

2. **Feature Similarity (30%)**: Cosine similarity of features
   - Counts operators, expression sizes, constants, variables
   - Uses vector space model

3. **Pattern Similarity (20%)**: High-level pattern matching
   - Counts function definitions, if statements, while loops, etc.
   - Helps detect algorithmic similarity

4. **Size Similarity (10%)**: Program length comparison
   - Penalizes significant size differences

## Testing

Local testing confirms:
- ✅ Parser correctly handles S-expressions
- ✅ Cheat mode produces transformed code
- ✅ Anticheat mode computes similarity scores
- ✅ Build system produces working `code` executable
- ✅ Code compiles cleanly with -Wall -Wextra

### Example Test
Input program:
```
(function (main)
  (block
    (set x 10)
    (print x)))
```

Transformed output:
```
(function (func_1) (block (set var_2 10) (print var_2)))
```

Similarity between original and transformed: 0.955926 (correctly identifies as highly similar)

## Build Instructions

The OJ evaluation system will:
1. Clone the repository
2. Run `cmake .` (if CMakeLists.txt exists) - optional
3. Run `make` (will use either existing Makefile or cmake-generated one)
4. Use the resulting `code` executable

## Execution Modes

The `code` executable supports:
- **Cheat mode**: `./code cheat < input.p` - transforms program
- **Anticheat mode**: `./code anticheat < two_programs.txt` - computes similarity

## Known Issues

- **OJ Submission**: Unable to submit via API due to problem configuration
  - Error: "unable to create submission"
  - Problem 1939 has `languages: []` (no languages configured)
  - This appears to be an OJ configuration issue, not a code issue

## Repository Status

All code has been committed and pushed to GitHub:
- Commit 3a5fbe2: Initial implementation with original Makefile
- Commit 0e3aa44: Added CMakeLists.txt
- Commit 82bd4d1: Restored Makefile for direct make support (CURRENT)

The repository is publicly accessible and ready for OJ evaluation once submissions are enabled.

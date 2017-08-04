# Get in the conversation

Join us on Slack at [hoppergh.slack.com](https://hoppergh.slack.com)!
(Sign-up form to come)

# Who can contribute

We welcome contribution from anyone interested in making this project better.
Although we intend all contributions to be high-quality code, we will help you
improve your code to match our standards if you have difficulties.

# How to get started

- Fork the repository on GitHub.
- Make sure you have all the dependencies:
  - clang && LLVM >= 3.8
  - gflags
  - clang-format 3.8 (this version to ensure there's no useless whitespace
    changes)
  - If you want to run coverage locally, you'll need:
    - python
    - lcov
    - genhtml
- Create your feature branch (usually based on master).
- Hack away!

## Coding style

The compiler is written in C++14. We use mostly the [Google style
guide](https://google.github.io/styleguide/cppguide.html), but with slightly
different naming conventions. All types (except standard ones) are in
_CamelCase_, all functions and variables in _snake\_case_.

# Where to get started

You want to contribute but you don't know what to write? Hop on the slack and
we'll help you. Alternatively, you can have a look at the list of
[issues](https://github.com/nitnelave/hopper/issues) or at the
[roadmap](https://docs.google.com/document/d/1g5gcPcA5aGNlQFjl1zEVmrlYCQGddJaoFBVIIg4OSQU).

# Making a Pull Request (PR)

After implementing your kick-ass feature, you want to create a PR to get your
code included in the main repository. For that you need to:

- Make sure `make check` still passes.
- Create logically separated commits (more on that later).
- Write tests for your feature.
- Make sure `make check` still passes.
- Check that the clang-tidy checks pass (`./tools/clang-tidy.sh`).
- Enforce our formatting guidelines on your code with `./tools/clang-format.sh`
- Create the pull request on GitHub. Reference any issue you are closing in the
  title.

When you create the PR, the CI will check that it builds correctly, passes all
the tests, that clang-tidy doesn't complain, and that the formatting is
correct. Then it will check the same thing on the state of master if it merged
your PR. Code coverage will be computed as well, and you will see the
difference in code coverage that your PR brings. If it is positive, it is
awesome, but if it is slightly negative it is okay (as long as your code is
well tested). Once everything is green, you can ask for a review (but you can
always ask for help!).

After that, a core contributor will have a look at your PR and review it on
reviewable.io (integrated with GitHub). If you make a series of valuable
contributions and are otherwise involved in the community, you can quickly
become a core contributor!

## A word on commits

Although a PR is reviewed and accepted as a whole, we still expect good quality
at the commit level. That includes:


### Small commits (but logically whole).
Commits should encompass a single, logical change. What does that mean? If you
are declaring a new function to use in a code path, then the function
declaration, definition and usage are a single, logical change. If you
introduce a new algorithm with a new data structure, then first introduce the
data structure (unused) in a commit, then the algorithm using it. Separate
style changes, refactoring, bug fixing and new features.

### Don't break the test suite (even at intermediate steps)

The idea is to be able to use `git bisect` if needed, but that relies on `make
check` passing at every commit. Sadly, for now we have no tool to enforce this,
so we only ask a best effort.

### Good commit messages.

A good starting point to write a good commit message is this article: [A Note
About Git Commit
Messages](http://tbaggery.com/2008/04/19/a-note-about-git-commit-messages.html).
However, we have a few small differences:

- We start the summary with one word on the "category" of the commit: ast,
  parsing, tests, doc, style, typechecking, ... Look at the git log for
  inspiration.
- We don't capitalize the summary (not critical, but let's keep it consistent).

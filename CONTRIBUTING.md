---
title: Contributing to G4VG
---

Thank you for your interest in G4VG; we welcome your contributions!

> [!NOTE]
> This contribution page is derived from the [Celeritas contribution document](https://celeritas-project.github.io/celeritas/user/development/contributing.html)

# Copyright

All submissions to the G4VG project are automatically licensed under the
terms of [the project copyright](LICENSE.md) as formalized by the
[GitHub terms of
service](https://docs.github.com/en/github/site-policy/github-terms-of-service#6-contributions-under-repository-license).
The person who writes a line of code is its author and copyright holder,
but the new code should be the shared responsibility of the project
rather than the exclusive property of a single contributor. G4VG is a
collaborative project with *collective* ownership.

# Attribution

You will get public credit for your work: your username and pull
requests will be listed in the release notes, *and* you will receive
official co-authorship on the Zenodo code object for the next major (and
minor, if your fix is backported) releases that incorporate your
contribution. By contributing, you acknowledge the attribution and
authorship policy laid out in `authorship`.

> [!NOTE]
> To uniquely link your contribution for the next release, you must add
> your ORCID to `scripts/release/users.json` to your first PR.
> (Otherwise, only your GitHub-derived name will show on the record.) If
> you want the release to be automatically propagated to your ORCID
> account, you must sign up for and connect it to
> [OpenAIRE](https://www.openaire.eu/openaire-explore-integration-with-the-orcid-search-and-link-wizard).

# Collaborating

Working with other team members is a critical part of the
development process. Please join the [G4VG Mattermost
channel](https://mattermost.web.cern.ch/celer-adept/channels/g4vg) or open an issue with a bug report or feature request.

# Development prerequisites

Create a
[fork](https://docs.github.com/en/pull-requests/collaborating-with-pull-requests/working-with-forks/about-forks)
of the G4VG repository. You should clone this fork locally to your
development machine as the "origin", and it's a good idea to add the
main `G4VG-project` repository as an "upstream" so that you can merge
changes from the main codebase as you work.

To meet the formatting requirements, you **must** install the [pre-commit](https://pre-commit.com)
utility (available with pip, homebrew, spack, and other package managers) using
`pre-commit install`.

A mostly consistent set of decorations (separators, Doxygen comment
structure, etc.) are used throughout the code, so try to make new files
look like existing ones. Use the
[celeritas-gen.py](https://github.com/celeritas-project/celeritas/blob/develop/scripts/dev/celeritas-gen.py)
script to generate skeletons for new files, and use existing source code as a
guide for how to structure the decorations.

# Submitting code changes

When you believe that you've made a substantive[^1] and self-contained
improvement to the code, it's time to create a [pull
request](https://docs.github.com/en/pull-requests/collaborating-with-pull-requests/proposing-changes-to-your-work-with-pull-requests/about-pull-requests)
(PR) to get feedback on your changes before they're merged into the code
base. The pull request should be as close to a "single change" as
possible (i.e., the short pull request title can essentially describe
the entire change set), typically a few hundred lines (not including
tests and test data). A pull request could be as small as a single line
for a bug fix. If your changes involve both substantial refactoring and
new features, try to split the refactoring into a separate commit.

Before opening the pull request, check that the [code](https://celeritas-project.github.io/celeritas/user/development/coding.html)
and [style](https://celeritas-project.github.io/celeritas/user/development/style.html) guidelines from Celeritas have been followed for all new
code and code changes. Ensure the use of the correct formatting as well
as the addition of documentation and unit tests for new code and bug
fixes.

After opening the pull request, the
[pre-commit.ci](https://pre-commit.ci) bot will run automatically and
check your formatting. If it fails, you probably did not install the
utility mentioned in the previous section. You can `git pull` the
suggested changes and then install `pre-commit`; or, if you've made
changes locally, then install pre-commit, you can execute
`pre-commit run --from-ref upstream/develop --to-ref HEAD` and
`git push -f` to update all files and force push.

All tests must pass on the CI runner before a PR can be merged. It's
best to test locally first before submitting your pull request, and keep
in mind that the multiple configurations on the CI (different dependency
versions, different features) may reveal failures that your local
testing might have missed.

Each pull request should be assigned one or two reviewers who will
provide constructive feedback on your changes. Reviews should not be started until the
"draft" status has been removed (if you marked it as a draft initially).
Once a pull request is under review, *do not* rebase, squash, or
otherwise alter the branch history. Such changes can drastically
increase the difficulty of reviewing, because it may blend in a single
commit both changes in response to a review *and* changes from upstream
code. (Furthermore, it breaks GitHub notifications and makes it more
difficult to find older comments.) You *can* merge the main upstream
branch if your changes may interact with the upstream changes, and you
*must* merge if they conflict.

The review is complete and your branch will be squashed and merged when:

- All the CI tests pass,
- All conversations have been resolved[^2], and
- The reviewer has approved the changes.

And you will officially be a G4VG contributor!  Congratulations!

[^1]: All changes to the codebase must go through the pull request, but
    due to the overhead of reviewing, testing, merging, and documenting
    a PR, we'd like to avoid small changes that have almost no effect in
    terms of operation or readability. For example, if you find a typo
    in the documentation, check the rest of the docs for any other typos
    or improvements you'd like to make, and submit a single PR with
    those changes.

[^2]: When you've fully implemented the reviewer's comment, you may mark
    it as resolved without commenting. Do not resolve a conversation if
    you disagree with the feedback: instead, post your view in a
    follow-on comment and wait for the reviewer to respond. If you
    comment, whether to supplement your change or to iterate with the
    reviewer, please do not resolve the conversation since that makes it
    hard to find your comment.

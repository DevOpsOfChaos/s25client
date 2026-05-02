# Chaos Edition Upstream Intake

Chaos Edition is an unofficial fork/product line of Return To The Roots. `upstream/master` remains the technical base, but it is not the product gatekeeper for Chaos Edition. Upstream changes should be reviewed, classified, and imported deliberately.

Do not blind-merge upstream into `chaos-edition`. The cost of a fast merge is paid later as unclear behavior changes, harder conflict resolution, and a fork that cannot explain what it actually chose.

## Intake Routine

Fetch upstream regularly:

```sh
git fetch upstream
```

Inspect new upstream commits:

```sh
git log --oneline chaos-edition..upstream/master
```

Review every listed commit before importing anything. Classify each commit as one of:

- `import`: bring the commit into Chaos Edition as-is.
- `partial-port`: port only the useful neutral part, usually by manual edit or a narrow cherry-pick.
- `skip`: do not import because it conflicts with Chaos Edition goals or is irrelevant.
- `later`: defer because it needs more context, testing, or dependent work.

## Import Branches

Create a temporary import branch for each intake session:

```sh
git switch chaos-edition
git switch -c chaos/import-upstream-YYYY-MM-DD
```

Do not experiment directly on `chaos-edition`. The branch should stay stable enough that maintainers can return to it, release from it, or compare it against upstream without sorting through unfinished import work.

Prefer cherry-pick or manual porting over a blind merge. A merge is only acceptable when the imported range is understood and the result has been reviewed as a curated product decision, not just as a Git convenience.

## Applying Changes

For each upstream commit:

- Read the commit and nearby code before applying it.
- Apply `import` commits with cherry-pick when practical.
- Apply `partial-port` commits manually when the upstream commit mixes neutral fixes with behavior that should stay upstream-only or conflicts with Chaos-only work.
- Leave `skip` commits unapplied and document why.
- Revisit `later` commits when their blocker is resolved.

After an import branch is ready, run the relevant build and tests before merging it back to `chaos-edition`. The exact command set depends on the touched area, but the standard is simple: do not merge an intake branch that has not been exercised against the risk it introduces.

## Intake Log

Keep an intake log for each session. The log can live in the import branch description, a pull request body, a maintainer note, or a tracked document when the intake is large enough to justify it.

Each entry should include:

- Date.
- Upstream commit.
- Decision: `import`, `partial-port`, `skip`, or `later`.
- Rationale.
- Result.

Example:

```text
Date: 2026-05-02
Upstream commit: abc1234 Fix deterministic replay crash
Decision: import
Rationale: Neutral crash fix, no Chaos-only behavior affected.
Result: Cherry-picked cleanly; replay smoke test passed.
```

The point is not paperwork. The point is to make future conflict resolution faster and to avoid pretending that every upstream change is automatically right for this fork.

## Sending Fixes Back Upstream

If a neutral fix originates in Chaos Edition and would be useful for upstream, extract it into a clean sidequest branch and open a small upstream pull request. Keep the branch focused on the neutral fix only. Do not mix it with Chaos-only gameplay, graphics, content, balance, AI, naming, or identity work.

Chaos-only gameplay, graphics, content, and AI changes stay in the fork unless they are deliberately redesigned as neutral upstream changes. That redesign is a separate decision, not something that happens by accident during upstream intake.

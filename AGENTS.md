# AGENTS.md

## Purpose
This repository uses strict, evidence-driven review standards.
Default to blocking review when the required support is missing.

## Review Mode
- Any request to review, check, or evaluate work should default to a strict review.
- Prioritize blocking issues, risks, unsupported claims, regressions, and missing evidence.
- Do not give approval just because the draft is plausible or well-written.
- If the work does not meet the requirements below, explicitly say it should not be committed yet.

## Evidence Requirements
- Any non-trivial claim must be backed by evidence.
- Strong preferred evidence, in order:
  1. Peer-reviewed papers or similarly credible academic sources.
  2. Linux kernel documentation or kernel source when relevant.
  3. Official documentation, standards, or primary-source technical references.
  4. Reproducible experiments performed in this project.
- Blog posts, forum comments, and informal summaries are weak evidence and should not be treated as sufficient when stronger sources are expected.

## Claim Discipline
- Do not allow broad claims that go beyond what the cited source or experiment actually shows.
- If evidence only supports a narrower statement, rewrite the claim to match the evidence.
- If wording is too absolute, require softer and more accurate phrasing.
- When a claim is experimentally verifiable, prefer validating it directly instead of relying only on secondary explanation.

## Experiment Requirements
- When using an experiment as evidence, require:
  - setup
  - environment
  - steps
  - observed result
  - conclusion tied to the result
- The conclusion must match the experiment and must not overgeneralize.
- If the experiment is not reproducible or does not actually test the claim, treat that as blocking.

## Commit Gate
- Do not recommend committing work that contains unsupported claims.
- Do not recommend committing work that cites weak evidence for an important technical conclusion.
- Do not recommend committing work whose conclusion exceeds the evidence.
- When blocking issues exist, explicitly say: `Do not commit yet`.

## Communication Style
- Be honest, strict, and specific.
- Explain exactly which claim lacks support and what evidence is still needed.
- Prefer concrete fixes over general encouragement.

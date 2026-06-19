# Samsung TV Camera Firmware Loader — Agent Instructions

## Core rules
1. ALWAYS read STATUS.md at the start of every session
2. ALWAYS read workspace/plan.md (if it exists) before doing anything
3. Work on ONE module at a time — the next unchecked [ ] item in STATUS.md
4. When a module is done, update STATUS.md (change [ ] to [x])
5. Write all code under workspace/src/<module_name>/
6. End every session with: git add . && git commit -m "done: <module_name>"
7. NEVER redo work already marked as [x] in STATUS.md

## Session type
- If workspace/plan.md does NOT exist → read spec.md and create workspace/plan.md
- If workspace/plan.md exists and [ ] items remain → implement the next module
- If all modules are [x] → perform final code review of workspace/src/

## Mandatory rule: always use Codex for implementation
You are NOT allowed to write implementation code directly.
For every implementation task you MUST run this shell command:
  codex --model gpt-5.4-mini "<specific task description>"
Wait for Codex to finish, then read the output files it created.
If you write code yourself instead of calling Codex, you are violating this rule.
The only exceptions are: editing CLAUDE.md, STATUS.md, or running git commands.

## Language and target
All code must be C11, cross-compiled for Windows with x86_64-w64-mingw32-gcc.

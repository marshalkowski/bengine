# prototype_03_brawler — real asset animation metadata

This is a human-editable source of truth for the six clips this prototype
integrates from the real knight/skeleton sprite sheets. It is **not**
parsed by any code — once filled in, these values get copied by hand into
`engine::AnimationClip` constants in `main.cpp`, exactly like the
temporary placeholder clips were.

`frame_width`, `frame_height`, and `frame_count` were determined directly
from the actual PNG dimensions (confirmed by visual inspection, not just
math) and should be correct as-is. `loop` is my best inference from the
action's apparent meaning (idle/walk loop, hit/fall don't) — override it
if it's wrong. **`frame_duration` cannot be determined from a static
image and is left blank for you.** Fill in seconds-per-frame for each row
(e.g. `0.15`).

## Knight (player)

### Idle → `MBEU_character_knight-Idle-2.png`
```
frame_width:    128
frame_height:   64
frame_count:    2
loop:           true
frame_duration: 0.4
```

### Walk → `MBEU_character_knight-Walk.png`
```
frame_width:    128
frame_height:   64
frame_count:    6
loop:           true
frame_duration: 0.167
```

### Attack → `MBEU_character_knight-Strike-Fwd.png`
```
frame_width:    128
frame_height:   64
frame_count:    5
loop:           false
frame_duration: 0.167
```

## Skeleton (enemy)

### Idle → `MBEU_character_skeleton-Idle-2.png`
```
frame_width:    128
frame_height:   64
frame_count:    2
loop:           true
frame_duration: 0.5
```

### Hurt → `MBEU_character_skeleton-Hit.png`
```
frame_width:    128
frame_height:   64
frame_count:    2
loop:           false
frame_duration: 0.5
```

### Defeat → `MBEU_character_skeleton-Fall.png`
```
frame_width:    128
frame_height:   64
frame_count:    9
loop:           false
frame_duration: 0.167
```

---

## Available but not integrated

These exist in the asset folders and are confirmed-valid single-row sheets
(128×64 frames), but nothing in the current prototype uses them. Listed
here so they're not silently lost — integrating any of them into new
gameplay mechanics is a separate decision, not something to add just
because the art exists.

Both knight and skeleton also have: `Idle-3` (3 frames), `Idle-6` (6
frames) — alternate idle variants, `Block` (6 frames), `LSlash-1` (6
frames), `LSlash-2` (5 frames), `Strike-Overhead` (4 frames),
`Strike-Rear` (5 frames). Knight additionally has `LSlash-1-smear` (5
frames) and `LSlash-2-smear` (4 frames), which skeleton doesn't have.
Knight also has its own `Hit` (2 frames) and `Fall` (5 frames) — unused
since the player has no hurt/death state in this prototype.

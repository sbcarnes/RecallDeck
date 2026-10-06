# RecallDeck
A native Win32 spaced-repetition flashcard application written in C.

The project is intended as both a practical study tool and as a
learning environment for custom Win32 controls, drawing, input
handling, persistence, and application state.

## Current Status

RecallDeck is currently a functional study prototype

Implemented features include:

- Custom-drawn flashcards with hover, click, and drag interaction.
- Front/back card review.
- Missed / Got It grading.
- External JSON deck loading.
- Randomized review order.
- Persistent per-card hit and miss counts.
- Session completion and restart.
- Progress reset with confirmation.
- Diagnostic display for deck and score state.

## Next Direction

The next development phase is adaptive review behavior.

RecallDeck currently records card performance but does not yet use that
performance to influence review order. The immediate goal is to derive
review weight from each card's history and use that information when
constructing review sessions.

Longer term, this will develop into true spaced-repetition scheduling
with cards becoming due based on review history and elapsed time.

## Planned Features

- Score-aware review ordering.
- Spaced-repetition scheduling.
- Card creation and editing.
- Deck selection and organization.
- Keyboard-oriented study workflow.
- Search and filtering.
- Import and export.
- Review statistics.

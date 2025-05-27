# `timetrack`

`timetrack` is a utility to log file changes---particularly `file open` and `file close` and derive meaning usage statistics.

### Features

- Automatically detect git repositories and track the repository's name
- Store timestamps when files in a `tracked` directories are accessed, modified, or closed.
- Visualizations and graphs providing statistics about your daily, weekly, monthly usage
- git-trackable `.timetrackdb` text file---you never lose your usage statistics
- All your favorite shell aliases work

### How to use

```bash
  > timetrack [command]
  ```
  
- Example:
```bash
  > timetrack nvim main.rs
  ```

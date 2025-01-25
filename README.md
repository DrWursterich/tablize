
# tablize

a very simple formatter for markdown tables. \
reads from stdin and prints to stdout

## features

consider the file `bad_table.md`:
```markdown
| key | value | description |
| --- | --- | --- |
| FRESH | 0 | nothing has happend yet |
| FIELD | 1 | parsing a field |
| NEW_LINE | 2 | beginning of a line |
| ERROR | 3 | something failed |
```

if we pipe it through tablize we recieve the following table:

```sh
cat bad_table.md | tablize
```

```markdown
| key      | value | description             |
| :------- | ----: | :---------------------- |
| FRESH    |     0 | nothing has happend yet |
| FIELD    |     1 | parsing a field         |
| NEW_LINE |     2 | beginning of a line     |
| ERROR    |     3 | something failed        |
```

## build

```sh
sudo make clean install
```

## uninstall

```sh
sudo make clean uninstall
```


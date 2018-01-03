# JSON Comment Pretty Print Helper
Need a better name for this

# What it does
Strips comments from JSON, or puts them back in, in separate steps, so that you can pretty print in between.
```
Usage: ./json_comment --help: generates this message
       ./json_comment --strip stripped_comments_file: strip comments and save to stripped_comments_file
       ./json_comment --apply stripped_comments_file: apply comments from stripped_comments_file
Reads json from stdin and writes json to stdout
```

# Limitations
This is currently in a proof-of-concept state, and so only supports ASCII input and C++-style comments. Those limitations can be removed if this overall
approach is feasible for a more generalizable tool.

The hardest limitation to address is how to retain comments while introducing a filter (e.g. jq). That might require integration with the filter code.

# Example
Assume my-ugly-commented.json is a JSON file that contains C++-style comments and that is not pretty-printed.
To pretty print using jq and retain your comments:
```
json_comment --strip my.comments < my-ugly-commented.json > my-ugly.json
jq '.' < my-ugly.json > my-pretty.json
json_comment --apply my.comments < my-pretty.json > my-pretty-commented.json
```

Note that using any filter other than '.' will yield meaningless garbage results. This limitation may require integration with jq to overcome.

# Build
cc -o json_comment json_comment.c

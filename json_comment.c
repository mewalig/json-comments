#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int read_to_delim_or_eof(FILE *f_in, FILE *f_out, const char delim) {
  int c;
  while((c = fgetc(f_in)) != EOF && c != delim) {
    if(f_out) {
      if(c == '\t')
        fwrite("\t", 1, 2, f_out);
      else if(c == '\n')
        fwrite("\n", 1, 2, f_out);
      else if(c == '\r')
        fwrite("\r", 1, 2, f_out);
      else
        fwrite(&c, 1, 1, f_out);
    }
  }
  return c;
}

#define JSON_COMMENT_MODE_STRIP 0
#define JSON_COMMENT_MODE_APPLY 1

static void usage(const char *s, FILE *f) {
  fprintf(f, "Usage: %s --help: generates this message\n"
             "       %s --strip stripped_comments_file: strip comments and save to stripped_comments_file\n"
             "       %s --apply stripped_comments_file: apply comments from stripped_comments_file\n"
          "  Reads json from stdin and writes json to stdout\n"
          , s, s, s);
}

struct comment {
  unsigned int i;
  char *line;
  unsigned int line_buff_len;
};

// get_next_comment: return zero if no next comment found
static char get_next_comment(FILE *comment_file, struct comment *next_comment) {
  unsigned int i = 0;
  unsigned int len = 0;
  next_comment->i = 0;
  next_comment->line[0] = '\0';

  int c;
  while((c = fgetc(comment_file)) != EOF) {
    if(c >= '0' && c <= '9')
      i = i*10 + (c - '0');
    else if(c == '\t') {
      while((c = fgetc(comment_file)) != EOF && c != '\n') {
        if(len < next_comment->line_buff_len - 1)
          next_comment->line[len++] = c;
      }
      next_comment->i = i;
      if(len)
        next_comment->line[len] = '\0';
      return (c != EOF);
    }
  }
  return 0;
}


#define JSON_COMMENT_MAX_LINE 16384

int main(int argc, char *argv[]) {
  if(argc > 1 && (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))) {
    usage(argv[0], stdout);
    return 0;
  }
  if(argc < 3 || !(!strcmp(argv[1], "--apply") || !strcmp(argv[1], "--strip"))) {
    usage(argv[0], stderr);
    return 0;
  }

  FILE *f_in = stdin;
  FILE *f_out = stdout;
  FILE *stripped_comments_file = NULL;

  struct comment next_comment;
  next_comment.line = NULL;

  char mode;
  char have_next_comment = 0;
  if(!strcmp(argv[1], "--apply")) {
    mode = JSON_COMMENT_MODE_APPLY;
    if(!(stripped_comments_file = fopen(argv[2], "rb")))
      fprintf(stderr, "Could not open file '%s' for reading", argv[2]);
    else {
      next_comment.line_buff_len = JSON_COMMENT_MAX_LINE;
      next_comment.line = malloc(next_comment.line_buff_len * sizeof(char));
      have_next_comment = get_next_comment(stripped_comments_file, &next_comment);
    }
  } else {
    mode = JSON_COMMENT_MODE_STRIP;
    if(!(stripped_comments_file = fopen(argv[2], "wb")))
      fprintf(stderr, "Could not open file '%s' for writing", argv[2]);
  }

  if(!stripped_comments_file) {
    usage(argv[0], stderr);
    return -1;
  }
  
  unsigned int element_count = 0;
  int c, prior_c = -1;
  
  do {
    c = fgetc(f_in);
    switch(c) {
    case ':':
    case '{':
    case '}':
    case '[':
    case ']':
      element_count++;
      break;
    case '"':
      element_count++;
      fwrite("\"", 1, 1, f_out);
      c = read_to_delim_or_eof(f_in, f_out, '"');
      break;
    case '/':
      if(mode == JSON_COMMENT_MODE_STRIP) {
        if(prior_c == '/') {
          fprintf(stripped_comments_file, "%u\t", element_count);
          c = read_to_delim_or_eof(f_in, stripped_comments_file, '\n');
          fwrite("\n", 1, 1, stripped_comments_file);
          prior_c = -1;
          // if(c != EOF)
          // c = fgetc(f_in);
        }
      }
      break;
    }

    prior_c = c;

    if(c != EOF) {
      if(c != '/')
        fwrite(&c, 1, 1, f_out);
      if(have_next_comment && element_count == next_comment.i) {
        fprintf(f_out, " //%s", next_comment.line);
        have_next_comment = get_next_comment(stripped_comments_file, &next_comment);
      }
    }
  } while(c != EOF);

  if(mode == JSON_COMMENT_MODE_APPLY)
    free(next_comment.line);

  fclose(stripped_comments_file);
  return 0;
}

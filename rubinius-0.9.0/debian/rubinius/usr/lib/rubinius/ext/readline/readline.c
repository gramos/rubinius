/* readline.c -- GNU Readline module
   Copyright (C) 1997-2001  Shugo Maeda */

#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "ruby.h"

#include <unistd.h>

static VALUE mReadline;

#define TOLOWER(c) (isupper(c) ? tolower(c) : c)

#define COMPLETION_PROC "completion_proc"
#define COMPLETION_CASE_FOLD "completion_case_fold"
static ID completion_proc, completion_case_fold;

static int readline_event(void);
static char **readline_attempted_completion_function(const char *text,
                                                     int start, int end);

static int
readline_event()
{
    CHECK_INTS;
    rb_thread_schedule();
    return 0;
}

static VALUE
readline_readline(VALUE self, VALUE tmp, VALUE add_hist)
{
    VALUE result;
    char *prompt = NULL;
    char *buff;
    int status;

    rb_secure(4);
    SafeStringValue(tmp);
    prompt = rb_str_get_char_ptr(tmp);

    if (!isatty(0) && errno == EBADF) rb_raise(rb_eIOError, "stdin closed");

    buff = readline(prompt);

#if 0
    buff = (char*)rb_protect((VALUE(*)_((VALUE)))readline, (VALUE)prompt,
                              &status);
    if (status) {
#if defined HAVE_RL_CLEANUP_AFTER_SIGNAL
        /* restore terminal mode and signal handler*/
        rl_cleanup_after_signal();
#elif defined HAVE_RL_DEPREP_TERM_FUNCTION
        /* restore terminal mode */
        (*rl_deprep_term_function)();
#else
        rl_deprep_terminal();
#endif
        rb_jump_tag(status);
    }
#endif

    if (RTEST(add_hist) && buff) {
      add_history(buff);
    }
    
    if (buff) {
      result = rb_tainted_str_new2(buff);
      free(buff);
    } else {
      result = Qnil;
    }
    
    return result;
}

static VALUE
readline_s_set_completion_proc(self, proc)
    VALUE self;
    VALUE proc;
{
    rb_secure(4);
    if (!rb_respond_to(proc, rb_intern("call")))
        rb_raise(rb_eArgError, "argument must respond to `call'");
    return rb_ivar_set(mReadline, completion_proc, proc);
}

static VALUE
readline_s_get_completion_proc(self)
    VALUE self;
{
    rb_secure(4);
    return rb_attr_get(mReadline, completion_proc);
}

static VALUE
readline_s_set_completion_case_fold(self, val)
    VALUE self;
    VALUE val;
{
    rb_secure(4);
    return rb_ivar_set(mReadline, completion_case_fold, val);
}

static VALUE
readline_s_get_completion_case_fold(self)
    VALUE self;
{
    rb_secure(4);
    return rb_attr_get(mReadline, completion_case_fold);
}

static char **
readline_attempted_completion_function(text, start, end)
    const char *text;
    int start;
    int end;
{
    VALUE proc, ary, temp;
    char **result;
    int case_fold;
    int i, matches;

    proc = rb_attr_get(mReadline, completion_proc);
    if (NIL_P(proc))
        return NULL;
#ifdef HAVE_RL_ATTEMPTED_COMPLETION_OVER
    rl_attempted_completion_over = 1;
#endif
    case_fold = RTEST(rb_attr_get(mReadline, completion_case_fold));
    ary = rb_funcall(proc, rb_intern("call"), 1, rb_tainted_str_new2(text));
    ary = rb_Array(ary);
    
    matches = rb_ary_size(ary);
    if (matches == 0) return NULL;
    result = ALLOC_N(char *, matches + 2);
    for (i = 0; i < matches; i++) {
      temp = rb_obj_as_string(rb_ary_entry(ary, i)); 
      result[i + 1] = rb_str_get_char_ptr(temp);
    }
    result[matches + 1] = NULL;

    if (matches == 1) {
        result[0] = strdup(result[1]);
    }
    else {
        register int i = 1;
        int low = 100000;

        while (i < matches) {
            register int c1, c2, si;

            if (case_fold) {
                for (si = 0;
                     (c1 = TOLOWER(result[i][si])) &&
                         (c2 = TOLOWER(result[i + 1][si]));
                     si++)
                    if (c1 != c2) break;
            } else {
                for (si = 0;
                     (c1 = result[i][si]) &&
                         (c2 = result[i + 1][si]);
                     si++)
                    if (c1 != c2) break;
            }

            if (low > si) low = si;
            i++;
        }
        result[0] = ALLOC_N(char, low + 1);
        strncpy(result[0], result[1], low);
        result[0][low] = '\0';
    }

    return result;
}

static VALUE
readline_s_set_completion_append_character(self, str)
    VALUE self, str;
{
    rb_secure(4);
    if (NIL_P(str)) {
        rl_completion_append_character = '\0';
    }
    else {
        SafeStringValue(str);
        if (rb_str_get_char_len(str) == 0) {
            rl_completion_append_character = '\0';
        } else {
            rl_completion_append_character = rb_str_get_char(str, 0);
        }
    }
    return self;
}

static VALUE
readline_s_get_completion_append_character(self)
    VALUE self;
{
    VALUE str;

    rb_secure(4);
    if (rl_completion_append_character == '\0')
        return Qnil;

    return rb_str_new((char*)&rl_completion_append_character, 1);
}

static VALUE
readline_s_set_basic_word_break_characters(self, str)
    VALUE self, str;
{

    rb_secure(4);
    SafeStringValue(str);
    
    rl_basic_word_break_characters = rb_str_get_char_ptr(str);
    
    return self;
}

static VALUE
readline_s_get_basic_word_break_characters(self, str)
    VALUE self, str;
{
    rb_secure(4);
    if (rl_basic_word_break_characters == NULL)
        return Qnil;
    return rb_tainted_str_new2(rl_basic_word_break_characters);
}

static VALUE
readline_s_set_completer_word_break_characters(self, str)
    VALUE self, str;
{
    rb_secure(4);
    SafeStringValue(str);
    
    rl_completer_word_break_characters = rb_str_get_char_ptr(str);
    
    return self;
}

static VALUE
readline_s_get_completer_word_break_characters(self, str)
    VALUE self, str;
{
    rb_secure(4);
    if (rl_completer_word_break_characters == NULL)
        return Qnil;
    return rb_tainted_str_new2(rl_completer_word_break_characters);
}

static VALUE
readline_s_set_completer_quote_characters(self, str)
    VALUE self, str;
{
    static char *completer_quote_characters = NULL;

    rb_secure(4);
    SafeStringValue(str);
    
    rl_completer_quote_characters = rb_str_get_char_ptr(str);

    return self;
}

static VALUE
readline_s_get_completer_quote_characters(self, str)
    VALUE self, str;
{
    rb_secure(4);
    if (rl_completer_quote_characters == NULL)
        return Qnil;
    return rb_tainted_str_new2(rl_completer_quote_characters);
}

static VALUE
hist_to_s(self)
    VALUE self;
{
    return rb_str_new2("HISTORY");
}

static VALUE
hist_get(self, index)
    VALUE self;
    VALUE index;
{
    HIST_ENTRY *entry;
    int i;

    rb_secure(4);
    i = NUM2INT(index);
    if (i < 0) {
        i += history_length;
    }
    entry = history_get(history_base + i);
    if (entry == NULL) {
        rb_raise(rb_eIndexError, "invalid index");
    }
    return rb_tainted_str_new2(entry->line);
}

static VALUE
hist_set(self, index, str)
    VALUE self;
    VALUE index;
    VALUE str;
{
    HIST_ENTRY *entry;
    int i;
    char *data;

    rb_secure(4);
    i = NUM2INT(index);
    SafeStringValue(str);
    if (i < 0) {
        i += history_length;
    }
    data = rb_str_get_char_ptr(str);
    entry = replace_history_entry(i, data, NULL);
    free(data);
    if (entry == NULL) {
        rb_raise(rb_eIndexError, "invalid index");
    }
    return str;
}

static VALUE
hist_push(self, str)
    VALUE self;
    VALUE str;
{
  char *data;
  rb_secure(4);
  SafeStringValue(str);
  data = rb_str_get_char_ptr(str);
  add_history(data);
  free(data);
  return self;
}

static VALUE
hist_push_method(argc, argv, self)
    int argc;
    VALUE *argv;
    VALUE self;
{
    VALUE str;
    char *data;
    
    rb_secure(4);
    while (argc--) {
        str = *argv++;
        SafeStringValue(str);
        data = rb_str_get_char_ptr(str);
        add_history(data);
        free(data);
    }
    return self;
}

static VALUE
rb_remove_history(index)
    int index;
{
    HIST_ENTRY *entry;
    VALUE val;

    rb_secure(4);
    entry = remove_history(index);
    if (entry) {
        val = rb_tainted_str_new2(entry->line);
        free((char*)entry->line);
        free(entry);
        return val;
    }
    return Qnil;
}

static VALUE
hist_pop(self)
    VALUE self;
{
    rb_secure(4);
    if (history_length > 0) {
        return rb_remove_history(history_length - 1);
    } else {
        return Qnil;
    }
}

static VALUE
hist_shift(self)
    VALUE self;
{
    rb_secure(4);
    if (history_length > 0) {
        return rb_remove_history(0);
    } else {
        return Qnil;
    }
}

static VALUE
hist_each(self)
    VALUE self;
{
    HIST_ENTRY *entry;
    int i;

    rb_secure(4);
    for (i = 0; i < history_length; i++) {
        entry = history_get(history_base + i);
        if (entry == NULL)
            break;
        rb_yield(rb_tainted_str_new2(entry->line));
    }
    return self;
}

static VALUE
hist_length(self)
    VALUE self;
{
    rb_secure(4);
    return INT2NUM(history_length);
}

static VALUE
hist_empty_p(self)
    VALUE self;
{
    rb_secure(4);
    return history_length == 0 ? Qtrue : Qfalse;
}

static VALUE
hist_delete_at(self, index)
    VALUE self;
    VALUE index;
{
    int i;

    rb_secure(4);
    i = NUM2INT(index);
    if (i < 0)
        i += history_length;
    if (i < 0 || i > history_length - 1) {
        rb_raise(rb_eIndexError, "invalid index");
    }
    return rb_remove_history(i);
}

void
Init_readline()
{
    VALUE history, fcomp, ucomp;

    /* Allow conditional parsing of the ~/.inputrc file. */
    rl_readline_name = "Ruby";

    using_history();

    completion_proc = rb_intern(COMPLETION_PROC);
    completion_case_fold = rb_intern(COMPLETION_CASE_FOLD);

    mReadline = rb_define_module("Readline");
    rb_define_singleton_method(mReadline, "perform_readline",
                              readline_readline, 2);
    rb_define_singleton_method(mReadline, "completion_proc=",
                               readline_s_set_completion_proc, 1);
    rb_define_singleton_method(mReadline, "completion_proc",
                               readline_s_get_completion_proc, 0);
    rb_define_singleton_method(mReadline, "completion_case_fold=",
                               readline_s_set_completion_case_fold, 1);
    rb_define_singleton_method(mReadline, "completion_case_fold",
                               readline_s_get_completion_case_fold, 0);
    rb_define_singleton_method(mReadline, "completion_append_character=",
                               readline_s_set_completion_append_character, 1);
    rb_define_singleton_method(mReadline, "completion_append_character",
                               readline_s_get_completion_append_character, 0);
    rb_define_singleton_method(mReadline, "basic_word_break_characters=",
                               readline_s_set_basic_word_break_characters, 1);
    rb_define_singleton_method(mReadline, "basic_word_break_characters",
                               readline_s_get_basic_word_break_characters, 0);
    rb_define_singleton_method(mReadline, "completer_word_break_characters=",
                               readline_s_set_completer_word_break_characters, 1);
    rb_define_singleton_method(mReadline, "completer_word_break_characters",
                               readline_s_get_completer_word_break_characters, 0);
    rb_define_singleton_method(mReadline, "completer_quote_characters=",
                               readline_s_set_completer_quote_characters, 1);
    rb_define_singleton_method(mReadline, "completer_quote_characters",
                               readline_s_get_completer_quote_characters, 0);

    history = rb_obj_alloc(rb_cObject);
    rb_define_singleton_method(history,"to_s", hist_to_s, 0);
    rb_define_singleton_method(history,"[]", hist_get, 1);
    rb_define_singleton_method(history,"[]=", hist_set, 2);
    rb_define_singleton_method(history,"<<", hist_push, 1);
    rb_define_singleton_method(history,"push", hist_push_method, -1);
    rb_define_singleton_method(history,"pop", hist_pop, 0);
    rb_define_singleton_method(history,"shift", hist_shift, 0);
    rb_define_singleton_method(history,"each", hist_each, 0);
    rb_define_singleton_method(history,"length", hist_length, 0);
    rb_define_singleton_method(history,"size", hist_length, 0);
    rb_define_singleton_method(history,"empty?", hist_empty_p, 0);
    rb_define_singleton_method(history,"delete_at", hist_delete_at, 1);
    rb_define_const(mReadline, "HISTORY", history);

    rb_define_const(mReadline, "VERSION", rb_str_new2(rl_library_version));

    rl_attempted_completion_function = readline_attempted_completion_function;
}

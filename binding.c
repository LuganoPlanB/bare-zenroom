#include <assert.h>
#include <bare.h>
#include <js.h>
#include <stddef.h>
#include <stdlib.h>
#include <utf.h>
#include <zenroom.h>

enum {
  bare_addon_stdout_capacity = 1024 * 1024,
  bare_addon_stderr_capacity = 256 * 1024
};

static char *
bare_addon_copy_string(js_env_t *env, js_value_t *value) {
  int err;

  size_t len = 0;
  err = js_get_value_string_utf8(env, value, NULL, 0, &len);
  assert(err == 0);

  char *result = calloc(len + 1, sizeof(char));
  assert(result != NULL);

  err = js_get_value_string_utf8(env, value, (utf8_t *) result, len + 1, &len);
  assert(err == 0);

  return result;
}

static js_value_t *
bare_addon_exec(js_env_t *env, js_callback_info_t *info) {
  int err;

  size_t argc = 6;
  js_value_t *argv[6];
  err = js_get_callback_info(env, info, &argc, argv, NULL, NULL);
  assert(err == 0);
  assert(argc == 6);

  char *script = bare_addon_copy_string(env, argv[0]);
  char *conf = bare_addon_copy_string(env, argv[1]);
  char *keys = bare_addon_copy_string(env, argv[2]);
  char *data = bare_addon_copy_string(env, argv[3]);
  char *extra = bare_addon_copy_string(env, argv[4]);
  char *context = bare_addon_copy_string(env, argv[5]);

  char *stdout_buf = calloc(bare_addon_stdout_capacity, sizeof(char));
  char *stderr_buf = calloc(bare_addon_stderr_capacity, sizeof(char));
  assert(stdout_buf != NULL);
  assert(stderr_buf != NULL);

  int exit_code = zencode_exec_tobuf(
    script,
    conf,
    keys,
    data,
    extra,
    context,
    stdout_buf,
    bare_addon_stdout_capacity,
    stderr_buf,
    bare_addon_stderr_capacity
  );

  js_value_t *result;
  err = js_create_object(env, &result);
  assert(err == 0);

  js_value_t *exit_code_value;
  err = js_create_int32(env, exit_code, &exit_code_value);
  assert(err == 0);
  err = js_set_named_property(env, result, "exitCode", exit_code_value);
  assert(err == 0);

  js_value_t *stdout_value;
  err = js_create_string_utf8(env, (utf8_t *) stdout_buf, -1, &stdout_value);
  assert(err == 0);
  err = js_set_named_property(env, result, "stdout", stdout_value);
  assert(err == 0);

  js_value_t *stderr_value;
  err = js_create_string_utf8(env, (utf8_t *) stderr_buf, -1, &stderr_value);
  assert(err == 0);
  err = js_set_named_property(env, result, "stderr", stderr_value);
  assert(err == 0);

  free(stdout_buf);
  free(stderr_buf);
  free(context);
  free(extra);
  free(data);
  free(keys);
  free(conf);
  free(script);

  return result;
}

static js_value_t *
bare_addon_exports(js_env_t *env, js_value_t *exports) {
  int err;

#define V(name, fn) \
  { \
    js_value_t *val; \
    err = js_create_function(env, name, -1, fn, NULL, &val); \
    assert(err == 0); \
    err = js_set_named_property(env, exports, name, val); \
    assert(err == 0); \
  }

  V("exec", bare_addon_exec)
#undef V

  return exports;
}

BARE_MODULE(bare_addon, bare_addon_exports)

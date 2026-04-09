#include <assert.h>
#include <bare.h>
#include <js.h>
#include <stddef.h>
#include <utf.h>

static js_value_t *
bare_addon_exec(js_env_t *env, js_callback_info_t *info) {
  int err;

  size_t argc = 6;
  js_value_t *argv[6];
  err = js_get_callback_info(env, info, &argc, argv, NULL, NULL);
  assert(err == 0);
  (void) argc;
  (void) argv;

  js_value_t *result;
  err = js_create_object(env, &result);
  assert(err == 0);

  js_value_t *exit_code;
  err = js_create_int32(env, 1, &exit_code);
  assert(err == 0);
  err = js_set_named_property(env, result, "exitCode", exit_code);
  assert(err == 0);

  js_value_t *stdout_value;
  err = js_create_string_utf8(env, (utf8_t *) "", -1, &stdout_value);
  assert(err == 0);
  err = js_set_named_property(env, result, "stdout", stdout_value);
  assert(err == 0);

  js_value_t *stderr_value;
  err = js_create_string_utf8(
    env,
    (utf8_t *) "Zenroom native library is not linked yet",
    -1,
    &stderr_value
  );
  assert(err == 0);
  err = js_set_named_property(env, result, "stderr", stderr_value);
  assert(err == 0);

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

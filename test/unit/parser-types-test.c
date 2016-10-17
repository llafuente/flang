/*
* Copyright 2015 Luis Lafuente <llafuente@noboxout.com>
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
* THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "../tasks.h"
#include "../test.h"

void test_parser_type(ast_t* body, size_t typeid) {
  ast_t* var_type;

  ASSERT(body->type == AST_DTOR_VAR, "first in body: AST_DTOR_VAR");

  var_type = body->var.type;
  ASSERT(var_type != 0, "dtor has a type");
  ASSERTE(var_type->type, AST_TYPE, "%d != %d", "dtor type is AST_TYPE");
  ASSERTE(var_type->ty_id, typeid, "%zu != %zu", "typeid ?");
}

// TODO more robust test... some values will change when core grow
TASK_IMPL(parser_types) {
  log_debug_level = 0;

  TEST_PARSER_OK("bool type=2", "var bool hello;",
                 { test_parser_type(*body, 2); });

  TEST_PARSER_OK("i8 type=3", "var i8 hello;",
                 { test_parser_type(*body, TS_I8); });

  TEST_PARSER_OK("u8 type=4", "var u8 hello;",
                 { test_parser_type(*body, TS_U8); });

  TEST_PARSER_OK("i16 type=5", "var i16 hello;",
                 { test_parser_type(*body, TS_I16); });

  TEST_PARSER_OK("i32 type=7", "var i32 hello;",
                 { test_parser_type(*body, TS_I32); });

  TEST_PARSER_OK("i64 type=9", "var i64 hello;",
                 { test_parser_type(*body, TS_I64); });

  TEST_PARSER_OK("f32 type=11", "var f32 hello;",
                 { test_parser_type(*body, TS_F32); });

  TEST_PARSER_OK("f64 type=12", "var f64 hello;", {
    test_parser_type(*body, TS_F64);

    // this must be tested inside, test macro clean types
    ty_t ty = ts_type_table[TS_F64];
    ASSERTE(ty.number.fp, true, "%d == %d", "type if fp");
    ASSERTE(ty.number.bits, 64, "%d == %d", "type is 64 bits");
  });

  TEST_PARSER_OK("custom type 01", "var ptr(f32) hello;",
                 { test_parser_type(*body, TEST_TYPEID); });

  TEST_PARSER_OK("uniques 01", "var ptr(f32) a; var ptr(f32) b;", {
    test_parser_type(body[0], TEST_TYPEID);
    test_parser_type(body[1], TEST_TYPEID);
  });

  TEST_PARSER_OK("inference 01", "var x; x = 10;",
                 { test_parser_type(body[0], TS_I64); });

  TEST_PARSER_ERROR("empty struct", "struct test {}",
                    "syntax error, empty struct declaration",
                    {/*CHK_ERROR_RANGE(err, 4, 1, 5, 1);*/
                    });

  TEST_PARSER_OK("simple struct", "struct test {"
                                  "i8 t1"
                                  "}",
                 { ASSERT(body[0]->ty_id != 0, "typeid is set"); });

  TEST_PARSER_OK("simple struct", "struct stt {"
                                  "i8 b"
                                  "}; struct stt2 {"
                                  "i8 b"
                                  "};",
                 {
    ASSERT(body[0]->type == AST_DECL_STRUCT, "AST_DECL_STRUCT");
    ASSERT(body[1]->type == AST_DECL_STRUCT, "AST_DECL_STRUCT");
  });

  TEST_PARSER_OK("complex struct", "struct test {"
                                   "i8 t1,"
                                   "i32 t2"
                                   "}",
                 { ASSERT(body[0]->ty_id == TEST_TYPEID, "typeid"); });

  TEST_PARSER_OK("struct unique with same type/props", "struct test {"
                                                       "i8 t1,"
                                                       "i32 t2"
                                                       "}"
                                                       "struct test2 {"
                                                       "i8 t1,"
                                                       "i32 t2"
                                                       "}",
                 {
    ASSERT(body[0]->ty_id == TEST_TYPEID, "typeid struct 1");
    ASSERT(body[1]->ty_id == TEST_TYPEID, "typeid struct 2");
  });

  TEST_PARSER_OK("struct unique with same type/props", "struct test {"
                                                       "i8 t1,"
                                                       "i32 t2"
                                                       "}"
                                                       "struct test2 {"
                                                       "i8 t1,"
                                                       "i32 t3"
                                                       "}",
                 {
    ASSERT(body[0]->ty_id == TEST_TYPEID, "typeid struct 1");
    ASSERT(body[1]->ty_id == TEST_TYPEID + 1, "typeid struct 2");
  });

  TEST_PARSER_OK("struct unique with same type/props", "struct test {"
                                                       "i8 t1,"
                                                       "i32 t2"
                                                       "}"
                                                       "struct test2 {"
                                                       "i8 t1,"
                                                       "i8 t2"
                                                       "}",
                 {
    ASSERT(body[0]->ty_id == TEST_TYPEID, "typeid struct 1");
    ASSERT(body[1]->ty_id == TEST_TYPEID + 1, "typeid struct 2");
  });

  TEST_PARSER_OK("struct type usage", "struct test {"
                                      "i8 t1,"
                                      "i32 t2"
                                      "}"
                                      "struct test2 {"
                                      "test t"
                                      "}",
                 {
    ASSERT(body[0]->ty_id == TEST_TYPEID, "typeid struct 1");
    ASSERT(body[1]->ty_id == TEST_TYPEID + 1, "typeid struct 2");
  });
  TEST_PARSER_OK("struct with vectors", "struct str {"
                                        "  i32 uniqueness,"
                                        "  i32 length,"
                                        "  i32 used,"
                                        "  u32 capacity,"
                                        "  i8 encoding,"
                                        "  vector(i8) value"
                                        "};",
                 { ASSERT(body[0]->ty_id == TEST_TYPEID, "typeid struct"); });
  TEST_PARSER_OK("void*", "var ptr(void) a;",
                 { ASSERT(body[0]->ty_id == TS_PVOID, "typeid ptr<void>"); });

  TEST_PARSER_OK("alloc", "fn alloc(u64 amount_of_bytes) : ptr(void) {}",
                 {// TODO search core.alloc and check both types are the same
                  // ASSERT(body[0]->ty_id == 18, "typeid struct");
                 });

  // TODO syntax: 'ptr<ptr<void>>' should be preferred?!
  TEST_PARSER_OK("void*", "var ptr(ptr(void)) a;", {
    ASSERT(body[0]->ty_id == TEST_TYPEID, "typeid ptr(void)");
  });

  /*
  // TODO uncomment this test, should pass
  TEST_PARSER_OK("string", "var string x;",
                 { ASSERT(body[0]->ty_id == TS_STRING, "typeid string"); });
 */

  TEST_PARSER_ERROR("name collision 01", "function a() {};\n"
                                         "struct a { i8 b, };",
                    "Type name 'a' in use by another type, previously defined "
                    "at memory:string:1:1",
                    {});

  TEST_PARSER_ERROR("name collision 02", "struct a { i8 b, };\n"
                                         "function a() {};",
                    "Function name 'a' in use by a type, previously defined at "
                    "memory:string:1:1",
                    {});

  TEST_PARSER_ERROR("name collision 03", "var i8 a;\n"
                                         "struct a { i8 b, };",
                    "Type name 'a' in use by a variable, previously defined at "
                    "memory:string:1:1",
                    {});

  TEST_PARSER_ERROR("name collision 04", "var i8 a;\n"
                                         "function a() {};",
                    "Function name 'a' in use by a variable, previously "
                    "defined at memory:string:1:1",
                    {});

  TEST_PARSER_ERROR("name collision 05", "{ global i8 a = 1; }"
                                         "function a() {};",
                    "Function name 'a' in use by a variable, previously "
                    "defined at memory:string:1:3",
                    {});

  TEST_PARSER_ERROR("empty struct", "struct a { i8 b, };\n"
                                    "struct b { i8 c, };\n"
                                    "var a _a;\n"
                                    "var b _b;\n"
                                    "_b = _a;\n",
                    "type error, invalid cast: types are not castables (struct "
                    "a { i8 b, }) to (struct b { i8 c, })",
                    {});

  TEST_PARSER_OK("a/b are the same type", "struct a { i8 b, };\n"
                                          "struct b { i8 b, };\n"
                                          "var a _a;\n"
                                          "var b _b;\n"
                                          "_b = cast(b) _a;\n",
                 {
    u64 a_ty_id = body[0]->ty_id;
    u64 b_ty_id = body[1]->ty_id;
    u64 vara_ty_id = body[2]->ty_id;
    u64 varb_ty_id = body[3]->ty_id;
    ASSERT(a_ty_id == vara_ty_id, "typeid ptr(void)");
    ASSERT(b_ty_id == varb_ty_id, "typeid ptr(void)");
    ASSERT(a_ty_id == b_ty_id, "typeid ptr(void)");
  });

  TEST_PARSER_OK("references", "var u64* p;\n"
                               "var ref(u64) p2;\n",
                 {
    u64 a_ty_id = body[0]->ty_id;
    u64 b_ty_id = body[1]->ty_id;
    ASSERT(a_ty_id == b_ty_id, "declared as ref or c-ptr, same type");
  });

  TEST_PARSER_OK("references", "var u64* p;\n"
                               "var ptr(u64) p2;\n",
                 {
    u64 a_ty_id = body[0]->ty_id;
    u64 b_ty_id = body[1]->ty_id;
    ASSERT(a_ty_id != b_ty_id, "ref and pointers aren't the same type");
  });

  TEST_PARSER_ERROR("a/b are the same type", "struct a { i32 b, };\n"
                                             "var a $a;\n"
                                             "var i8 $b = 100;\n"
                                             "$a = cast(a) $b;\n",
                    "type error, invalid cast: types are not castables (i8) to "
                    "(struct a { i32 b, })",
                    {});

  TEST_PARSER_OK("implement type when used",
                 "template $tpl;\n"
                 "struct a($tpl) { i32 b, ptr($tpl) c };\n"
                 "var a(i8) a_instance;",
                 {
    // NOTE you my expect id=2, but the struct is duplicated
    // var statement is the fourth
    u64 var_ty_id = body[3]->ty_id;
    ty_t type = ty(var_ty_id);
    ASSERT(type.structure.members.length == 2, "2 members");
    ASSERT(type.structure.fields[0] == TS_I32, "first is I32");
    ty_t ptr_ty = ty(type.structure.fields[1]);
    ASSERT(ptr_ty.of == TY_POINTER, "second is a pointer");
    ASSERT(ptr_ty.ptr.to == TS_I8, "ptr to I8");
  });

  TEST_PARSER_ERROR("incomplete struct", "struct a { i32 b, xx };\n",
                    "syntax error, unexpected '}', expecting identifier", {});

  TEST_PARSER_ERROR("incomplete struct", "struct { i32 b, };\n",
                    "syntax error, unexpected '{', expecting identifier", {});

  TEST_PARSER_ERROR(
      "invalid member access", "struct st { i32 b, };\n"
                               "var st sti;\n"
                               "sti.b;\n"
                               "sti.c;\n",
      "type error, invalid member access 'c' for struct: struct st { i32 b, }",
      {});

  TEST_PARSER_ERROR(
      "function property 01", "function property first(i8 arr) {\n"
                              "  return 0;\n"
                              "}\n",
      "function property first argument must be a struct given: i8", {});

  TEST_PARSER_ERROR("function property 02",
                    "struct arr {i8 a};\n"
                    "function property first(arr ari) {\n"
                    "  return 0;\n"
                    "}\n"
                    "function property first(arr ari) {\n"
                    "  return 0;\n"
                    "}\n",
                    "function property redefinition (same name), previously "
                    "defined at memory:string:2:1",
                    {});

  TEST_PARSER_ERROR(
      "function property 03", "function property first() {\n"
                              "  return 0;\n"
                              "}\n",
      "syntax error, struct property overloading must have an unique parameter",
      {});

#define V2_DECL "struct v2 {\nf32 x,\nf32 y,\n};\n"

  TEST_PARSER_ERROR("function property 03",
                    V2_DECL "function operator +(v2 a) : v2 {"
                            "  return a;"
                            "}",
                    "syntax error, operator require 2 parameters only", {});

  TEST_PARSER_ERROR("function property 03",
                    V2_DECL "function operator +(v2 a, i8 x, i8b) : v2 {"
                            "  return a;"
                            "}",
                    "syntax error, operator require 2 parameters only", {});

  TEST_PARSER_ERROR("pointer arithmetic", "struct str {i8 xx}; var str _str;\n"
                                          "var ptr(i16) a;\n"
                                          "a = a + _str;",
                    "type error, invalid operands for pointer arithmetic\n"
                    "left is (ptr(i16)) but right is not numeric or pointer is "
                    "(struct str { i8 xx, }).",
                    {});

  TEST_PARSER_ERROR("pointer arithmetic", "struct str {i8 xx}; var str _str;\n"
                                          "var ptr(i16) a;\n"
                                          "a = _str + a;",
                    "type error, invalid operands for pointer arithmetic\n"
                    "right is (ptr(i16)) but left is not numeric or pointer is "
                    "(struct str { i8 xx, }).",
                    {});

  /* TODO this may raise an error in the future when string struct is ready
  TEST_PARSER_ERROR("pointer arithmetic",
                    "var ptr(i8) a;"
                    "a = \"string!\" + a;",
                    "type error, invalid operands for pointer arithmetic\n"
  "left is (ptr(i8)) but right is not numeric or pointer is (ptr(i8)).", {});
  */

  TEST_PARSER_ERROR("dereference", "var i16 b;\n"
                                   "*b;",
                    "type error, cannot dereference type (i16), must be a "
                    "pointer or a reference",
                    {});

  TEST_PARSER_ERROR("negation", "struct test {"
                                "i8 t1"
                                "}"
                                "var test t;\n"
                                "if (!t) {}",
                    "type error, invalid argument type (struct test { i8 t1, "
                    "}) for logical negation operator",
                    {});

  TEST_PARSER_ERROR("cast number-struct explicit", "struct test {"
                                                   "i8 t1"
                                                   "}"
                                                   "var test t;\n"
                                                   "var i8 a = 10;\n"
                                                   "t = cast(test) a;",
                    "type error, invalid cast: types are not castables (i8) to "
                    "(struct test { i8 t1, })",
                    {});

  TEST_PARSER_ERROR("cast number-struct", "struct test {"
                                          "i8 t1"
                                          "}"
                                          "var test t;\n"
                                          "var i8 a = 10;\n"
                                          "t = a;",
                    "type error, invalid cast: types are not castables (i8) to "
                    "(struct test { i8 t1, })",
                    {});
  TEST_PARSER_ERROR(
      "cast number-struct", "struct test {"
                            "i8 t1"
                            "}"
                            "var test t = 10;\n",
      "type error, numeric type cannot be casted to (struct test { i8 t1, })",
      {});

  TEST_PARSER_ERROR("cast number-struct",
                    "struct v2x {f32 x, f32 y,};\n"
                    "function operator [](v2x a) : f32 {\n"
                    "  var ptr(f32) x = unsafe_cast(ptr(f32)) a;\n"
                    "  return x[index];\n"
                    "}\n",
                    "syntax error, operator require 2 parameters only", {});

  TEST_PARSER_ERROR("cast number-struct",
                    "struct v2x {f32 x, f32 y,};\n"
                    "function operator [](v2x a, u64 index) : f32 {\n"
                    "  var ptr(f32) x = unsafe_cast(ptr(f32)) a;\n"
                    "  return x[index];\n"
                    "}\n",
                    "type error, operator[] requires first parameter to be a "
                    "reference given (struct v2x { f32 x, f32 y, })",
                    {});

  TEST_PARSER_ERROR(
      "cast number-struct", "struct v2x {f32 x, f32 y,};\n"
                            "function operator []=(v2x* a, u64 index) : f32 {\n"
                            "  var ptr(f32) x = unsafe_cast(ptr(f32)) a;\n"
                            "  return x[index];\n"
                            "}\n",
      "type error, operator[]= requires to return a reference, returned (f32)",
      {});

  TEST_PARSER_OK("ptrdiff", "var ptr(i16) a;\n"
                            "var ptr(i16) b;\n"
                            "var ptrdiff c = a - b;\n",
                 {});

  TEST_PARSER_ERROR(
      "template a template", "template $tpl;\n"
                             "struct array($tpl) { $tpl* values, };\n"
                             "function x (array($tpl) arg1) {}\n",
      "type error, try to implement a template using another template", {});

  TEST_PARSER_OK("implement templates in order left to right",
                 "template $tpl;\n"
                 "struct array($tpl) { $tpl values, };\n"
                 "function x (array a, $tpl b) {}\n"
                 "var array(i8) ai8;\n"
                 "x(ai8, 10);\n",
                 {
    ast_t* call = body[6];
    ASSERT(call->type == AST_EXPR_CALL, "7th is the call");
    u64 ty_first = call->call.arguments->list.values[0]->ty_id;
    ty_t type_first = ty(ty_first);
    u64 ty_second = call->call.arguments->list.values[1]->ty_id;
    ASSERT(type_first.of == TY_STRUCT, "first is a struct");
    ASSERT(type_first.structure.members.length == 1, "one field");
    ASSERT(type_first.structure.fields[0] == TS_I8, "first subtype is i8");
    ASSERT(ty_second == TS_I8, "second type is i8");
  });

  TEST_PARSER_ERROR("check casting after implement a template",
                    "var i64 num64 = 10;\n"
                    "template $tpl;\n"
                    "struct array($tpl) { $tpl values, };\n"
                    "function x (array a, $tpl b) {}\n"
                    "var array(i8) ai8;\n"
                    "x(ai8, num64);\n",
                    "type error, explicit cast required between (i64) to (i8)",
                    {});

  TEST_PARSER_OK("implement templates in order left to right",
                 "var i64 num64 = 10;\n"
                 "template $tpl;\n"
                 "struct array($tpl) { $tpl values, };\n"
                 "function x (array a, $tpl b) {}\n"
                 "var array(i8) ai8;\n"
                 "x(ai8, cast(i8) num64);\n",
                 {
    ast_t* call = body[7];
    ASSERT(call->type == AST_EXPR_CALL, "8th is the call");
    u64 ty_first = call->call.arguments->list.values[0]->ty_id;
    ty_t type_first = ty(ty_first);
    u64 ty_second = call->call.arguments->list.values[1]->ty_id;
    ASSERT(type_first.of == TY_STRUCT, "first is a struct");
    ASSERT(type_first.structure.members.length == 1, "one field");
    ASSERT(type_first.structure.fields[0] == TS_I8, "first subtype is i8");
    ASSERT(ty_second == TS_I8, "second type is i8");
  });

  TEST_PARSER_ERROR(
      "type inside type", "struct st { i8 a, st b };\n",
      "type error, type cannot be determined before declaring the struct", {});

  TEST_PARSER_OK("type inside a type by ref", "struct st { i8 a, st* b };\n",
                 {});

  TEST_PARSER_OK("type inside a type by ref/ptr",
                 "struct st { i8 a, ptr(st) b };\n", {});

  TEST_PARSER_ERROR(
      "implement templates with another template",
      "template $tpl;\n"
      "struct array($tpl) { $tpl values, };\n"
      "implement array($tpl) as array_i8;\n",
      "type error, try to implement a template using another template", {});

  TEST_PARSER_ERROR(
      "var declaration with a template is forbidden", "template $tpl;\n"
                                                      "var ptr($tpl) x;\n",
      "type error, cannot declare a variable with a templated type", {});

  TEST_PARSER_ERROR(
      "var declaration with a template is forbidden", "template $tpl;\n"
                                                      "fn fntpl($tpl x) {}\n"
                                                      "var fntpl xy;\n",
      "type error, cannot declare a variable with a templated type", {});

  TEST_PARSER_ERROR("undefined type", "var fntpl xy;\n",
                    "type error, cannot determine type", {});

  TEST_PARSER_ERROR(
      "undefined funtion", "fntpl();\n",
      "type error, cannot find function or variable with given name: 'fntpl'",
      {});

  TEST_PARSER_ERROR("undefined variable", "var a = 1;\n"
                                          "var b = 2;"
                                          "b = a + c;",
                    "type error, cannot find declaration for: 'c'", {});

  TEST_PARSER_ERROR("implement a reference, requires a reference",
                    "template $tpl;\n"
                    "fn fntpl($tpl* x) {}\n"
                    "var i8 xy;\n"
                    "fntpl(xy);\n",
                    "type error, cannot implement type (ref()) into (i8). A "
                    "reference is required.",
                    {});

  TEST_PARSER_ERROR("implement a pointer, requires a pointer",
                    "template $tpl;\n"
                    "fn fntpl(ptr($tpl) x) {}\n"
                    "var i8* xy;\n"
                    "fntpl(xy);\n",
                    "type error, cannot implement type (ptr()) into (ref(i8)). "
                    "A pointer is required.",
                    {});

  TEST_PARSER_ERROR("implement a struct, requires a struct",
                    "template $tpl;\n"
                    "struct array($tpl) { $tpl values, };\n"
                    "fn fntpl(array x) {}\n"
                    "var u64 xy;\n"
                    "fntpl(xy);\n",
                    "type error, cannot implement type (struct array {  "
                    "values, }) into (u64). A struct is required.",
                    {});

  TEST_PARSER_OK("type inside a type by ref/ptr",
                 "template $tpl;\n"
                 "struct array($tpl) { $tpl values, };\n"
                 "implement array(i8) as array_i8;\n",
                 {
    // TODO this test depends on core :S
    ASSERTE(body[1]->ty_id, TEST_TYPEID + 2, "%d != %d", "type implemented");
  });

  return 0;
}

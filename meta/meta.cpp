#include <stdio.h>
#include <string.h>

#include <tree_sitter/api.h>

#if defined(__unix__) || defined(__APPLE__)
#include <dirent.h>
#endif

#include "def.hpp"
#include "allocator.hpp"
#include "arena.hpp"
#include "array.hpp"
#include "string.hpp"

struct Field_Data
{
  char *field_ptr;
  u32 field_len;

  char *str_ptr;
  u32 str_len;
};

struct Enum_Data
{
  char *enum_ptr;
  u32 enum_len;

  u32 max_desc_len;

  Array<Field_Data> fields;

  Enum_Data(Allocator *alloc)
  {
    fields = Array<Field_Data>(alloc);
    enum_ptr = nullptr;
    enum_len = 0;
    max_desc_len = 0;
  }
};

extern "C" const TSLanguage *tree_sitter_cpp(void);

void print_max_field_func(FILE *fp, u32 len, String8 enum_name);

void stringify_extract(TSNode node, char *src, Array<Enum_Data>& arr)
{
  Enum_Data enum_data(arr.alloc);

  const char *type = ts_node_type(node);

  if (strcmp(type, "enum_specifier") == 0)
  {
    TSNode enum_name = ts_node_child_by_field_name(node, "name", 4);
    TSNode attr = ts_node_child_by_field_name(node, "attr", 4);
    TSNode enum_body = ts_node_child_by_field_name(node, "body", 4);

    if (ts_node_is_null(enum_name)) goto skip;
    if (ts_node_is_null(attr)) goto skip;
    if (ts_node_is_null(enum_body)) goto skip;
    u32 child_count = ts_node_named_child_count(enum_body);

    TSNode attribute = ts_node_named_child(attr, 0);
    const char *type = ts_node_type(attribute);
    if (strcmp(type, "attribute") != 0) goto skip;
    TSNode prefix = ts_node_child_by_field_name(attribute, "prefix", 6);
    TSNode name = ts_node_child_by_field_name(attribute, "name", 4);
    if (ts_node_is_null(prefix)) goto skip;
    if (ts_node_is_null(name)) goto skip;

    u32 prefix_start = ts_node_start_byte(prefix);
    u32 prefix_end = ts_node_end_byte(prefix);
    char *prefix_ptr = src + prefix_start;
    u32 prefix_len = prefix_end - prefix_start;

    u32 attr_start = ts_node_start_byte(name);
    u32 attr_end = ts_node_end_byte(name);
    char *attr_ptr = src + attr_start;
    u32 attr_len = attr_end - attr_start;

    if (strncmp(prefix_ptr, "meta", prefix_len) != 0) goto skip;
    if (strncmp(attr_ptr, "stringify", attr_len) != 0) goto skip;

    if (child_count > 0)
    {
      u32 start_byte = ts_node_start_byte(enum_name);
      u32 end_byte = ts_node_end_byte(enum_name);
      char *name_ptr = src + start_byte;
      u32 name_len = end_byte - start_byte;

      enum_data.enum_ptr = name_ptr;
      enum_data.enum_len = name_len;

      for (uint32_t i = 0; i < child_count; i++)
      {
        TSNode field = ts_node_named_child(enum_body, i);
        if (strcmp(ts_node_type(field), "meta_enumerator") == 0)
        {
          TSNode enum_field_name = ts_node_child_by_field_name(
            field, "name", 4
          );

          TSNode enum_field_tag = ts_node_child_by_field_name(
            field, "tag", 3
          );

          if (ts_node_is_null(enum_field_name)) continue;
          if (ts_node_is_null(enum_field_tag)) continue;

          u32 start_byte = ts_node_start_byte(enum_field_name);
          u32 end_byte = ts_node_end_byte(enum_field_name);
          char *field_ptr = src + start_byte;
          u32 field_len = end_byte - start_byte;

          TSNode tag_args = ts_node_child_by_field_name(
            enum_field_tag, "args", 4
          );

          if (ts_node_is_null(tag_args)) continue;

          TSNode tag_str = ts_node_named_child(tag_args, 0);
          TSNode str_content = ts_node_named_child(tag_str, 0);

          u32 str_start = ts_node_start_byte(str_content);
          u32 str_end = ts_node_end_byte(str_content);
          char *str_ptr = src + str_start;
          u32 str_len = str_end - str_start;

          if (str_len > enum_data.max_desc_len)
          {
            enum_data.max_desc_len = str_len;
          }

          Field_Data enum_field;
          enum_field.field_len = field_len;
          enum_field.field_ptr = field_ptr;
          enum_field.str_len = str_len;
          enum_field.str_ptr = str_ptr;

          enum_data.fields.append(enum_field);
        }
      }

      arr.append(enum_data);
    }
  }

  skip:
  u32 child_count = ts_node_child_count(node);
  for (u32 i = 0; i < child_count; i++)
  {
    stringify_extract(ts_node_child(node, i), src, arr);
  }
}

void print_max_field_func(FILE *fp, u32 len, String8 enum_name)
{
  fprintf(fp, "static int %s_max_field_len()\n", enum_name.c_str());
  fprintf(fp, "{\n");
  fprintf(fp, "  return %u;\n", len);
  fprintf(fp, "}\n");
  fprintf(fp, "\n");
}

int main(int argc, char *argv[])
{
  TSParser *parser = ts_parser_new();
  ts_parser_set_language(parser, tree_sitter_cpp());

  Arena arena(Arena::linked);

  #if defined(__unix__) || defined(__APPLE__)

  Array<Enum_Data> enums(&arena);

  DIR *dir = opendir("src");
  struct dirent *ent;
  while ((ent = readdir(dir)) != nullptr)
  {
    String8 file_name(ent->d_name, &arena);
    if (file_name.ends_in(".hpp"))
    {
      String8 full_path(&arena);
      full_path.cat("./src/");
      full_path.cat(ent->d_name);

      String8 file_buffer;
      String8::read_from_file(&file_buffer, full_path.c_str(), &arena);

      TSTree *tree = ts_parser_parse_string(parser, nullptr,
                                            file_buffer.c_str(),
                                            file_buffer.length);
      TSNode root = ts_tree_root_node(tree);

      String8 generated_path(&arena);
      generated_path.cat("./generated/");
      file_name.cut(4);
      generated_path.cat(file_name.c_str());
      generated_path.cat("_meta.hpp");

      stringify_extract(root, file_buffer.c_str(), enums);

      FILE *fp = fopen(generated_path.c_str(), "w");

      fprintf(fp, "#pragma once\n");
      fprintf(fp, "\n");

      fprintf(fp, "#include \"%s\"\n", ent->d_name);
      fprintf(fp, "\n");

      for (int i = 0; i < enums.length; i++)
      {
        Enum_Data curr_enum = enums[i];

        String8 enum_name(curr_enum.enum_ptr, curr_enum.enum_len, &arena);
        enum_name.to_lower();

        fprintf(fp, "enum struct %.*s : int;\n",
                curr_enum.enum_len, curr_enum.enum_ptr);

        fprintf(fp, "static const char *str_from_%s(%.*s e)\n",
                enum_name.c_str(),
                curr_enum.enum_len, curr_enum.enum_ptr);
        fprintf(fp, "{\n");
        fprintf(fp, "  switch (e)\n");
        fprintf(fp, "  {\n");

        for (int j = 0; j < curr_enum.fields.length; j++)
        {
          Field_Data curr_field = curr_enum.fields[j];
          fprintf(fp, "    case %.*s::%.*s:\n",
                  curr_enum.enum_len, curr_enum.enum_ptr,
                  curr_field.field_len, curr_field.field_ptr);
          fprintf(fp, "      return \"%.*s\";\n",
                  curr_field.str_len, curr_field.str_ptr);
        }

        fprintf(fp, "  }\n");
        fprintf(fp, "  return \"\";\n");
        fprintf(fp, "}\n");
        fprintf(fp, "\n");

        print_max_field_func(fp, curr_enum.max_desc_len, enum_name);
      }

      fclose(fp);
    }

    enums.reset();
  }

  arena.deinit();

  closedir(dir);

  #endif
}


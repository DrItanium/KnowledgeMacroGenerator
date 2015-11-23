This plugin is used to generate the list of fields for the knowledge
construction engine. 

Build the plugin by running `make` in this directory.

Once the plugin is built, you can run it using:
--
Linux:
$ clang -cc1 -load ../../Debug+Asserts/lib/libKnowledgeMacroGenerator.so -plugin print-fns some-input-file.c
$ clang -cc1 -load ../../Debug+Asserts/lib/libKnowledgeMacroGenerator.so -plugin print-fns -plugin-arg-print-fns help -plugin-arg-print-fns --example-argument some-input-file.c
$ clang -cc1 -load ../../Debug+Asserts/lib/libKnowledgeMacroGenerator.so -plugin print-fns -plugin-arg-print-fns -an-error some-input-file.c

Mac:
$ clang -cc1 -load ../../Debug+Asserts/lib/libKnowledgeMacroGenerator.dylib -plugin print-fns some-input-file.c
$ clang -cc1 -load ../../Debug+Asserts/lib/libKnowledgeMacroGenerator.dylib -plugin print-fns -plugin-arg-print-fns help -plugin-arg-print-fns --example-argument some-input-file.c
$ clang -cc1 -load ../../Debug+Asserts/lib/libKnowledgeMacroGenerator.dylib -plugin print-fns -plugin-arg-print-fns -an-error some-input-file.c

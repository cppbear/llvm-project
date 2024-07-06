// #include "CoutContext.h"

// // bool SignatureApplication::has(std::string file_path, std::string
// class_name,
// //                                std::string signature) {
// //   for (auto file : files) {
// //     if (file.file_path == file_path) {
// //       for (auto a_class : file.classes) {
// //         if (a_class.class_name == class_name) {
// //           for (auto method : a_class.signatures) {
// //             if (method == signature) {
// //               return 1;
// //             }
// //           }
// //           return 0;
// //         }
// //       }
// //       return 0;
// //     }
// //   }
// //   return 0;
// // }

// void SignatureApplication::append(std::string file_path, std::string
// class_name,
//                                   std::string signature) {
//   bool b = 0;
//   for (auto &file : files) {
//     if (file.file_path == file_path) {
//       for (auto &a_class : file.classes) {
//         if (a_class.class_name == class_name) {
//           a_class.signatures.insert(signature);
//           b = 1;
//         }
//       }
//       if (b == 0) {
//         SignatureClass new_signature_class;
//         new_signature_class.class_name = class_name;
//         new_signature_class.signatures.insert(signature);
//         file.classes.push_back(new_signature_class);
//         b = 1;
//       }
//     }
//   }
//   if (b == 0) {
//     SignatureFiles new_signature_file;
//     new_signature_file.file_path = file_path;
//     SignatureClass new_signature_class;
//     new_signature_class.class_name = class_name;
//     new_signature_class.signatures.insert(signature);
//     new_signature_file.classes.push_back(new_signature_class);
//     files.push_back(new_signature_file);
//   }
// }

// json SignatureContext::get_j() {
//   // std::string file_path;
//   // std::string signature;
//   // std::string class_name;
//   // std::string function_name;
//   // std::vector<std::string> includes;
//   // std::vector<ADefine> defines;
//   // std::vector<GlobalVar> global_vars;
//   // std::vector<Class> classes;
//   // std::vector<Function> functions;
//   json j = json::object();
//   j[file_path] = json::object();
//   j[file_path][class_name + "::" + function_name] = json::object();
//   j[file_path][class_name + "::" + function_name][signature] =
//   json::object(); j[file_path][class_name + "::" +
//   function_name][signature]["includes"] =
//       json::array();
//   j[file_path][class_name + "::" + function_name][signature]["includes"] =
//       includes;
//   j[file_path][class_name + "::" + function_name][signature]["defines"] =
//       json::object();
//   for (auto define : defines) {
//     j[file_path][class_name + "::" + function_name][signature]["defines"]
//      [define.define_name] = define.define_body;
//   }
//   for (auto var : global_vars) {
//     if (j[file_path][class_name + "::" + function_name][signature].find(
//             var.its_namespace) ==
//         j[file_path][class_name + "::" + function_name][signature].end()) {
//       j[file_path][class_name + "::" + function_name][signature]
//        [var.its_namespace] = json::object();
//     }
//     if (j[file_path][class_name + "::" + function_name][signature]
//          [var.its_namespace]
//              .find("global_vars") ==
//         j[file_path][class_name + "::" + function_name][signature]
//          [var.its_namespace]
//              .end()) {
//       j[file_path][class_name + "::" + function_name][signature]
//        [var.its_namespace]["global_vars"] = json::array();
//     }
//     j[file_path][class_name + "::" + function_name][signature]
//      [var.its_namespace]["global_vars"]
//          .push_back(var.global_var);
//   }
//   if (function_name != "TestBody") {
//     j[file_path][class_name + "::" + function_name][signature]
//      ["function_body"] = json::object();
//     j[file_path][class_name + "::" + function_name][signature]
//      ["function_body"] = function_body;
//   }
//   for (int i = 0; i < classes.size(); i++) {
//     if (j[file_path][class_name + "::" + function_name][signature].find(
//             classes[i].its_namespace) ==
//         j[file_path][class_name + "::" + function_name][signature].end()) {
//       j[file_path][class_name + "::" + function_name][signature]
//        [classes[i].its_namespace] = json::object();
//     }
//     if (j[file_path][class_name + "::" + function_name][signature]
//          [classes[i].its_namespace]
//              .find("class") == j[file_path][class_name + "::" +
//              function_name]
//                                 [signature][classes[i].its_namespace]
//                                     .end()) {
//       j[file_path][class_name + "::" + function_name][signature]
//        [classes[i].its_namespace]["class"] = json::object();
//     }
//     j[file_path][class_name + "::" + function_name][signature]
//      [classes[i].its_namespace]["class"][classes[i].class_name] =
//          json::object();
//     j[file_path][class_name + "::" + function_name][signature]
//      [classes[i].its_namespace]["class"][classes[i].class_name]["base_class"]
//      =
//          classes[i].base_class;
//     j[file_path][class_name + "::" + function_name][signature]
//      [classes[i].its_namespace]["class"][classes[i].class_name]["constructor"]
//      =
//          json::object();
//     for (int k = 0; k < classes[i].constructors.size(); k++) {
//       j[file_path][class_name + "::" + function_name][signature]
//        [classes[i].its_namespace]["class"][classes[i].class_name]["constructor"]
//        [classes[i].constructors[k].signature] = json::object();
//       j[file_path][class_name + "::" + function_name][signature]
//        [classes[i].its_namespace]["class"][classes[i].class_name]["constructor"]
//        [classes[i].constructors[k].signature]["function_body"] =
//            classes[i].constructors[k].function_body;
//       j[file_path][class_name + "::" + function_name][signature]
//        [classes[i].its_namespace]["class"][classes[i].class_name]["constructor"]
//        [classes[i].constructors[k].signature]["parameters"] = json::array();
//       j[file_path][class_name + "::" + function_name][signature]
//        [classes[i].its_namespace]["class"][classes[i].class_name]["constructor"]
//        [classes[i].constructors[k].signature]["parameters"] =
//            classes[i].constructors[k].parameters;
//     }
//     j[file_path][class_name + "::" + function_name][signature]
//      [classes[i].its_namespace]["class"][classes[i].class_name]["destructor"]
//      =
//          json::object();
//     j[file_path][class_name + "::" + function_name][signature]
//      [classes[i].its_namespace]["class"][classes[i].class_name]["destructor"]
//      ["signature"] = classes[i].destructor.signature;
//     j[file_path][class_name + "::" + function_name][signature]
//      [classes[i].its_namespace]["class"][classes[i].class_name]["destructor"]
//      ["function_body"] = classes[i].destructor.function_body;
//     j[file_path][class_name + "::" + function_name][signature]
//      [classes[i].its_namespace]["class"][classes[i].class_name]["fields"] =
//          json::array();
//     j[file_path][class_name + "::" + function_name][signature]
//      [classes[i].its_namespace]["class"][classes[i].class_name]["fields"] =
//          classes[i].fields;
//     j[file_path][class_name + "::" + function_name][signature]
//      [classes[i].its_namespace]["class"][classes[i].class_name]["methods"] =
//          json::object();
//     for (int k = 0; k < classes[i].methods.size(); k++) {
//       j[file_path][class_name + "::" + function_name][signature]
//        [classes[i].its_namespace]["class"][classes[i].class_name]["methods"]
//        [classes[i].methods[k].signature] = json::object();
//       j[file_path][class_name + "::" + function_name][signature]
//        [classes[i].its_namespace]["class"][classes[i].class_name]["methods"]
//        [classes[i].methods[k].signature]["function_body"] =
//            classes[i].methods[k].function_body;
//       j[file_path][class_name + "::" + function_name][signature]
//        [classes[i].its_namespace]["class"][classes[i].class_name]["methods"]
//        [classes[i].methods[k].signature]["parameters"] = json::array();
//       j[file_path][class_name + "::" + function_name][signature]
//        [classes[i].its_namespace]["class"][classes[i].class_name]["methods"]
//        [classes[i].methods[k].signature]["parameters"] =
//            classes[i].methods[k].parameters;
//       j[file_path][class_name + "::" + function_name][signature]
//        [classes[i].its_namespace]["class"][classes[i].class_name]["methods"]
//        [classes[i].methods[k].signature]["return_type"] =
//            classes[i].methods[k].return_type;
//     }
//   }
//   for (int i = 0; i < functions.size(); i++) {
//     if (j[file_path][class_name + "::" + function_name][signature].find(
//             functions[i].its_namespace) ==
//         j[file_path][class_name + "::" + function_name][signature].end()) {
//       j[file_path][class_name + "::" + function_name][signature]
//        [functions[i].its_namespace] = json::object();
//     }
//     if (j[file_path][class_name + "::" + function_name][signature]
//          [functions[i].its_namespace]
//              .find("function") ==
//         j[file_path][class_name + "::" + function_name][signature]
//          [functions[i].its_namespace]
//              .end()) {
//       j[file_path][class_name + "::" + function_name][signature]
//        [functions[i].its_namespace]["function"] = json::object();
//     }
//     j[file_path][class_name + "::" + function_name][signature]
//      [functions[i].its_namespace]["function"][functions[i].signature] =
//          json::object();
//     j[file_path][class_name + "::" + function_name][signature]
//      [functions[i].its_namespace]["function"][functions[i].signature]
//      ["function_body"] = functions[i].function_body;
//     j[file_path][class_name + "::" + function_name][signature]
//      [functions[i].its_namespace]["function"][functions[i].signature]
//      ["parameters"] = json::array();
//     j[file_path][class_name + "::" + function_name][signature]
//      [functions[i].its_namespace]["function"][functions[i].signature]
//      ["parameters"] = functions[i].parameters;
//     j[file_path][class_name + "::" + function_name][signature]
//      [functions[i].its_namespace]["function"][functions[i].signature]
//      ["return_type"] = functions[i].return_type;
//   }

//   return j;
// }

// void get_all_applications(std::vector<Application> *applications,
//                           std::vector<FileContext> &file_contexts,
//                           std::string file_path, std::string class_name,
//                           std::string signature) {
//   for (auto file_context : file_contexts) {
//     if (file_context.file_path == file_path) {
//       if (class_name == "class") {
//         for (auto function : file_context.functions) {
//           if (function.signature == signature) {
//             Application application;
//             application.class_name = class_name;
//             application.file_path = file_path;
//             application.signature = signature;
//             applications->push_back(application);
//             if (function.applications.size() > 0) {
//               for (auto application : function.applications) {
//                 bool b = 1;
//                 for (auto has_application : *applications) {
//                   if (has_application == application) {
//                     b = 0;
//                     break;
//                   }
//                 }
//                 if (b) {
//                   get_all_applications(
//                       applications, file_contexts, application.file_path,
//                       application.class_name, application.signature);
//                 }
//               }
//             }
//           }
//         }
//       } else {
//         for (auto a_class : file_context.classes) {
//           if (class_name == a_class.class_name) {
//             for (auto constructor : a_class.constructors) {
//               if (signature == constructor.signature) {
//                 Application application;
//                 application.class_name = class_name;
//                 application.file_path = file_path;
//                 application.signature = signature;
//                 applications->push_back(application);
//                 if (constructor.applications.size() > 0) {
//                   for (auto application : constructor.applications) {
//                     bool b = 1;
//                     for (auto has_application : *applications) {
//                       if (has_application == application) {
//                         b = 0;
//                         break;
//                       }
//                     }
//                     if (b) {
//                       get_all_applications(
//                           applications, file_contexts, application.file_path,
//                           application.class_name, application.signature);
//                     }
//                   }
//                 }
//               }
//             }
//             auto destructor = a_class.destructor;
//             if (signature == destructor.signature) {
//               Application application;
//               application.class_name = class_name;
//               application.file_path = file_path;
//               application.signature = signature;
//               applications->push_back(application);
//               if (destructor.applications.size() > 0) {
//                 for (auto application : destructor.applications) {
//                   bool b = 1;
//                   for (auto has_application : *applications) {
//                     if (has_application == application) {
//                       b = 0;
//                       break;
//                     }
//                   }
//                   if (b) {
//                     get_all_applications(
//                         applications, file_contexts, application.file_path,
//                         application.class_name, application.signature);
//                   }
//                 }
//               }
//             }
//             for (auto method : a_class.methods) {
//               if (signature == method.signature) {
//                 Application application;
//                 application.class_name = class_name;
//                 application.file_path = file_path;
//                 application.signature = signature;
//                 applications->push_back(application);
//                 if (method.applications.size() > 0) {
//                   for (auto application : method.applications) {
//                     bool b = 1;
//                     for (auto has_application : *applications) {
//                       if (has_application == application) {
//                         b = 0;
//                         break;
//                       }
//                     }
//                     if (b) {
//                       get_all_applications(
//                           applications, file_contexts, application.file_path,
//                           application.class_name, application.signature);
//                     }
//                   }
//                 }
//               }
//             }
//             break;
//           }
//         }
//       }
//       break;
//     }
//   }
// }

// SignatureContext get_signature_context(std::vector<FileContext>
// &file_contexts,
//                                        std::string file_path,
//                                        std::string class_name,
//                                        std::string signature) {
//   for (auto file_context : file_contexts) {
//     if (file_context.file_path == file_path) {
//       std::vector<Application> *applications = new std::vector<Application>;
//       get_all_applications(applications, file_contexts, file_path,
//       class_name,
//                            signature);
//       SignatureApplication signature_application;
//       for (auto application : *applications) {
//         signature_application.append(application.file_path,
//                                      application.class_name,
//                                      application.signature);
//       }
//       //   class SignatureContext {
//       //     std::string file_path;
//       //     std::string signature;
//       //     std::string class_name;
//       //     std::string function_name;
//       //     std::vector<std::string> includes;
//       //     std::vector<ADefine> defines;
//       //     std::vector<GlobalVar> global_vars;
//       //     std::vector<Class> classes;
//       //     std::vector<Function> functions;
//       //   };
//       SignatureContext signature_context;
//       signature_context.file_path = file_path;
//       signature_context.signature = signature;
//       signature_context.class_name = class_name;
//       for (auto file_context : file_contexts) {
//         if (file_context.file_path == file_path) {
//           signature_context.includes = file_context.includes;
//           signature_context.defines = file_context.defines;
//           signature_context.global_vars = file_context.global_vars;
//           if (class_name == "class") {
//             for (auto function : file_context.functions) {
//               if (function.signature == signature) {
//                 signature_context.function_name = function.function_name;
//                 signature_context.function_body = function.function_body;
//                 break;
//               }
//             }
//           } else {
//             bool b = 0;
//             for (auto a_class : file_context.classes) {
//               if (a_class.class_name == class_name) {
//                 for (auto method : a_class.constructors) {
//                   if (method.signature == signature) {
//                     signature_context.function_name = class_name;
//                     signature_context.function_body = method.function_body;
//                     b = 1;
//                     break;
//                   }
//                 }
//                 if (b == 0 && a_class.destructor.signature == signature) {
//                   signature_context.function_name = "~" + class_name;
//                   signature_context.function_body =
//                       a_class.destructor.function_body;
//                 }
//                 for (auto method : a_class.methods) {
//                   if (b == 1) {
//                     break;
//                   }
//                   if (method.signature == signature) {
//                     signature_context.function_name = method.method_name;
//                     signature_context.function_body = method.function_body;
//                     b = 1;
//                     break;
//                   }
//                 }
//                 break;
//               }
//             }
//           }
//           break;
//         }
//       }
//       for (auto file : signature_application.files) {
//         FileContext file_context;
//         for (int i = 0; i < file_contexts.size(); i++) {
//           if (file_contexts[i].file_path == file.file_path) {
//             file_context = file_contexts[i];
//             break;
//           }
//         }
//         for (auto a_class : file.classes) {
//           if (a_class.class_name == "class") {
//             for (auto signature : a_class.signatures) {
//               signature_context.functions.push_back(
//                   file_context.get_function(signature));
//             }
//           } else {
//             int i = 0;
//             for (i; i < signature_context.classes.size(); i++) {
//               if (signature_context.classes[i].class_name ==
//                   a_class.class_name) {
//                 break;
//               }
//             }
//             if (i == signature_context.classes.size()) {
//               signature_context.classes.push_back(
//                   file_context.get_simple_class(a_class.class_name));
//             }
//             for (auto signature : a_class.signatures) {
//               if (file_context.class_has_constructor(a_class.class_name,
//                                                      signature)) {
//                 for (int j = 0;
//                      j < signature_context.classes[i].constructors.size();
//                      j++) {
//                   if (signature_context.classes[i].constructors[j].signature
//                   ==
//                       signature) {
//                     signature_context.classes[i].constructors[j] =
//                         file_context.class_get_constructor(a_class.class_name,
//                                                            signature);
//                   }
//                 }
//               } else if
//               (file_context.class_has_destructor(a_class.class_name,
//                                                            signature)) {
//                 signature_context.classes[i].destructor =
//                     file_context.class_get_destructor(a_class.class_name,
//                                                       signature);
//               } else if (file_context.class_has_method(a_class.class_name,
//                                                        signature)) {
//                 for (int j = 0; j <
//                 signature_context.classes[i].methods.size();
//                      j++) {
//                   if (signature_context.classes[i].methods[j].signature ==
//                       signature) {
//                     signature_context.classes[i].methods[j] =
//                         file_context.class_get_method(a_class.class_name,
//                                                       signature);
//                   }
//                 }
//               }
//             }
//           }
//         }
//       }
//       delete applications;
//       return signature_context;
//     }
//   }
// }

// std::vector<TestMacro> get_may_tests(std::vector<FileContext> &file_contexts,
//                                      std::string class_name,
//                                      std::string function_name) {
//   std::vector<TestMacro> ret;
//   for (auto file_context : file_contexts) {
//     std::vector<TestMacro> file_may_test_macros =
//         file_context.get_may_test_macros(class_name, function_name);
//     for (auto test_macro : file_may_test_macros) {
//       ret.push_back(test_macro);
//     }
//   }
//   return ret;
// }

// std::vector<TestMacro> get_must_test(std::vector<FileContext> &file_contexts,
//                                      std::string second_parameter) {
//   std::vector<TestMacro> ret;
//   for (auto file_context : file_contexts) {
//     std::vector<TestMacro> file_must_test_macro =
//         file_context.get_must_test_macros(second_parameter);
//     if (file_must_test_macro.size() != 0) {
//       ret.push_back(file_must_test_macro[0]);
//       return ret;
//     }
//   }
//   return ret;
// }
var inspect = function() {
  return require("util").inspect(arguments[0], {depth: null, colors: true});
};

//console.log(process.argv);
//process.exit();

// npm install xml2js

var doc = {
  enum: {},
  struct: {},
  typedef: {},
  function: {}
};

function md_escape(text) {
  return text
  .replace(/\*/g, "\\*")
  .replace(/\_/g, "\\_");
}

function name(t) {
  return md_escape(t.$.name);
}

function type(t, escape) {
  var tt = t[0].$.name;

  if (t[0].$.qualifier) {
    switch (t[0].$.qualifier.trim()) {
    case "const *":
      tt = "const " + tt + "*";
      break;
    default:
      tt += " " + (t[0].$.qualifier || "");
    }
  }

  tt = tt.trim();

  if (escape) {
    tt = md_escape(tt);
  }

  return tt;
}

function brief(t, preprend_line) {
  preprend_line = preprend_line || "";

  return (t.brief || [])
  .concat(t.doc)
  .join("\n")
  .split("\n")
  .map(function(t) {
    return preprend_line + (t ? t.trim() : "");
  })
  .join("\n");
}

function doc_enum(en) {
  //console.error(inspect(en));

  var d = [
  //"<a name=\"" + en.$.name.replace(/\_/g, "\\_") + "\"></a>",
  "<a name=\"" + en.$.name + "\"></a>",
  "### enum " + name(en),
  "",
  brief(en)
  ];
  if (en.enumvalue && en.enumvalue.length) {
    d = d.concat([
      "",
      "##### Values",
      ""
    ]).concat(
      en.enumvalue.map(function(f, k) {
        return "* *" + name(f)  + "*\n\n" + brief(f, "  ");
      })
    );
  }

  doc.enum[en.$.name] = d.join("\n");
}

function doc_struct(st) {
  //console.error(inspect(st));

  var d = [
    "<a name=\"" + st.$.name + "\"></a>",
    "### struct " + name(st),
    "",
    brief(st),
  ];
  if (st.field) {
    d = d
    .concat([
      "",
      "##### Fields",
      ""
    ])
    .concat(
      (st.field).map(function(f, k) {
        return "* `" + type(f.type, false) + "` *" + name(f)  + "*\n\n" + brief(f, "  ");
      })
    );
  }

  doc.struct[st.$.name] = d.join("\n");
}

function doc_function(fn) {
  //console.error(inspect(fn));

  var fn_name = fn.$.name;
  if (doc.function[fn_name]) {
    return console.error("ignore function: ", fn_name);
  }
  var fn_brief = brief(fn);
  if (fn_brief.indexOf("@return") > -1) {
    throw new Error("error @" + fn_name);
  }

  fn_brief = fn_brief.replace("@alias", "");

  if (fn_brief.indexOf("@") > -1) {
    console.log("brief", fn_brief);
    throw new Error("brief constains @ bad parsed: " + fn_name);
  }

  var full_name = type(fn.return[0].type, true) + " " + name(fn);
  if (fn.argument && fn.argument.length) {
    full_name += "(" + fn.argument.map(function(a) {
      return type(a.type, true) + " " + a.$.name;
    }).join(", ") + ")";
  }

  var d = [
    "<a name=\"" + fn_name + "\"></a>",
    "### " + full_name,
    "",
    fn_brief
  ];

  var rdoc = brief(fn.return[0]);
  if (rdoc.length) {
    d = d.concat([
      "",
      "##### Return: " + type(fn.return[0].type, true),
      "",
      brief(fn.return[0])
    ]);
  }

  if (fn.argument && fn.argument.length) {
    //console.error(inspect(fn));
    d = d.concat([
      "",
      "##### Arguments (" + fn.argument.length + ")",
      "",
      fn.argument.map(function(a, k) {
        return "* `" + type(a.type) + "` *" + name(a)  + "*\n" + brief(a);
      }).join("\n")
    ]);
  }

  doc.function[fn_name] = d.join("\n");
}

function doc_typedef(tp) {
  //console.log(inspect(tp));

  var d = [
  "<a name=\"" + tp.$.name + "\"></a>",
  "### typedef " + name(tp) + " " + type(tp.type),
  "",
  brief(tp)
  ];

  doc.typedef[tp.$.name] = d.join("\n");
}

// TODO variable!
function docit(node) {
  if (node.function) {
    node.function.forEach(doc_function);
  }

  if (node.enum) {
    node.enum.forEach(doc_enum);
  }

  if (node.struct) {
    node.struct.forEach(doc_struct);
  }

  if (node.typedef) {
    node.typedef.forEach(doc_typedef);
  }
}

var parseString = require('xml2js').parseString;

files = process.argv.slice(2);
// index.xml should be last
var idx = null;
files = files.filter(function(file) {
  var found = file.indexOf("index.xml");
  if (found > -1) {
    idx = file;
    return false;
  }
  return true;
});
idx && files.push(idx);

files.forEach(function(file) {
  console.error("parsing: ", file);
  var xml = require("fs").readFileSync(file, {encoding: "utf-8"});
  //console.log(xml);
  parseString(xml, function (err, result) {
    //console.log(inspect(result));
    Object.keys(result).forEach(function(v, k) {
      switch(v) {
        case "struct":
          doc_struct(result[v]);
          docit(result[v]);
          return;
        case "typedef":
          doc_typedef(result[v]);
          docit(result[v]);
          return;
        case "category":
          //console.error(inspect(result[v]));
          docit(result[v]);
          return;
        case "index":
          //console.error(inspect(v));
          //console.error(inspect(result[v]));

          docit(result[v]);
          break;
        case "report":
          console.error("ignore report");
          break;
        default:
          console.error(v);
          throw new Error("WTF?!");
      }



      //console.log(inspect(result[v]));
    });
  });
});

console.log("# API");
console.log("## TOC");
["typedef", "enum", "struct", "function"].forEach(function(zone) {
  console.log("");
  console.log(zone);
  console.log("");

  Object.keys(doc[zone]).sort().forEach(function(k) {
    console.log("* [" + md_escape(k) + "](#" + k + ")");
  });

});


//console.error(inspect(doc));

[
  ["typedef", "Types"],
  ["enum", "Enums"],
  ["struct", "Structs"],
  ["function", "Functions"]
].forEach(function(d) {
  console.log("");
  console.log("## " + d[1]);
  console.log("");

  Object.keys(doc[d[0]]).sort().forEach(function(k) {
    console.log(doc[d[0]][k]);
    console.log("");
    console.log("---");
    console.log("");
  });
});

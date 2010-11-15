#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

#include <prscfl.h>


void 
dumpStructName(FILE *fh, ParamDef *def, char *delim) {
	if (def && def->parent && def->parent->name) {
		dumpStructName(fh, def->parent, delim);
		fputs(delim, fh);
		fputs(def->parent->name, fh);
	}
}

static void
dumpParamDef(FILE *fh, char* name, ParamDef *def) {
	
	if (def->comment) {
		ParamDef	*i = def->comment;

		if (i->next) {
			/* multiline comment */
			fprintf(fh, "\n\t/*\n");
			while(i) {
				fprintf(fh, "\t * %s\n", i->paramValue.commentval);
				i = i->next;
			}
			fprintf(fh, "\t */\n");
		} else {
			/* single line comment */
			fprintf(fh, "\n\t/* %s */\n", i->paramValue.commentval);
		}
	}

	switch(def->paramType) {
		case	int32Type:
			fprintf(fh, "\tint32_t\t%s;\n", def->name);
			break;
		case	uint32Type:
			fprintf(fh, "\tu_int32_t\t%s;\n", def->name);
			break;
		case	int64Type:
			fprintf(fh, "\tint64_t\t%s;\n", def->name);
			break;
		case	uint64Type:
			fprintf(fh, "\tu_int64_t\t%s;\n", def->name);
			break;
		case	doubleType:
			fprintf(fh, "\tdouble\t%s;\n", def->name);
			break;
		case	stringType:
			fprintf(fh, "\tchar*\t%s;\n", def->name);
			break;
		case	commentType:
			fprintf(stderr, "Unexpected comment"); 
			break;
		case	structType:
			fprintf(fh, "\t%s", name);
			dumpStructName(fh, def->paramValue.structval, "_");
			fprintf(fh, "*\t%s;\n", def->name);
			break;
		case	arrayType:
			fprintf(fh, "\t%s", name);
			dumpStructName(fh, def->paramValue.arrayval, "_");
			fprintf(fh, "**\t%s;\n", def->name);
			break;
		case 	builtinType:
			break;
		default:
			fprintf(stderr,"Unknown paramType (%d)\n", def->paramType);
			exit(1);
	}
}

static void
dumpParamList(FILE *fh, char* name, ParamDef *def) {
	while(def) {
		dumpParamDef(fh, name, def);
		def = def->next;
	}
}

static void
dumpStruct(FILE *fh, char* name, ParamDef *def) {
	ParamDef *list = NULL;

	switch(def->paramType) {
		case structType:
			list = def->paramValue.structval;
			break;
		case arrayType:
			list = def->paramValue.arrayval;
			break;
		default:
			fprintf(stderr,"Non-struct paramType (%d)\n", def->paramType);
			exit(1);
			break;
	}

	fprintf(fh, "typedef struct %s", name);
	dumpStructName(fh, list, "_");
	fputs(" {\n", fh);
	dumpParamList(fh, name, list);
	fprintf(fh, "} %s", name);
	dumpStructName(fh, list, "_");
	fputs(";\n\n", fh);
}

static void
dumpRecursive(FILE *fh, char* name, ParamDef *def) {

	while(def) {
		switch(def->paramType) {
			case structType:
				dumpRecursive(fh, name, def->paramValue.structval);
				dumpStruct(fh, name, def);
				break;
			case arrayType:
				dumpRecursive(fh, name, def->paramValue.arrayval);
				dumpStruct(fh, name, def);
				break;
			default:
				break;
		}

		def = def->next;
	}
}

void 
hDump(FILE *fh, char* name, ParamDef *def) {
	ParamDef	root;

	root.paramType = structType;
	root.paramValue.structval = def;
	root.name = NULL;
	root.parent = NULL;
	root.next = NULL;
	def->parent = &root;

	fprintf(fh, "#ifndef %s_CFG_H\n", name);
	fprintf(fh, "#define %s_CFG_H\n\n", name);
	
	fputs(
		"#include <stdio.h>\n"
		"#include <sys/types.h>\n\n"
		"/*\n"
		" * Autogenerated file, do not edit it!\n"
		" */\n\n",
		fh
	);

	dumpRecursive(fh, name, &root);

	fprintf(fh, "int fill_default_%s(%s *c);\n", name, name);
	fprintf(fh, "void parse_cfg_file_%s(%s *c, FILE *fh, int check_rdonly, int *n_accepted, int *n_skipped);\n\n", name, name);
	fprintf(fh, "void parse_cfg_buffer_%s(%s *c, char *buffer, int check_rdonly, int *n_accepted, int *n_skipped);\n\n", name, name);
	fprintf(fh, "int check_cfg_%s(%s *c);\n\n", name, name);
	fprintf(fh, "typedef struct %s_iterator_t %s_iterator_t;\n", name, name);
	fprintf(fh, "%s_iterator_t* %s_iterator_init();\n", name, name);
	fprintf(fh, "char* %s_iterator_next(%s_iterator_t* i, %s *c, char **v);\n", name, name, name);

	fputs("#endif\n", fh);

	def->parent = NULL;
}

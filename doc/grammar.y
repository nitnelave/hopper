%start dummy

%token END_OF_FILE
%token LOWER_CASE_IDENT
%token MACRO_IDENT
%token UPPER_CASE_IDENT
%token INT
%token HEX
%token OCT
%token BINARY_NUMBER
%token FLOAT
%token STRING
%token RAWSTRING
%token CODESTRING
%token RAWCODESTRING
%token PLUS
%token MINUS
%token DIVIDE
%token INTEGER_DIVIDE
%token MODULO
%token STAR
%token OR
%token AND
%token GREATER
%token LESS
%token GREATER_OR_EQUAL
%token LESS_OR_EQUAL
%token EQUAL
%token DIFFERENT
%token BITSHIFT_RIGHT
%token BITSHIFT_LEFT
%token BITOR
%token BITXOR
%token QUESTION_MARK_COLON
%token QUESTION_MARK_DOT
%token QUESTION_MARK_ARROW
%token ASSIGN
%token PLUS_ASSIGN
%token MINUS_ASSIGN
%token TIMES_ASSIGN
%token DIVIDE_ASSIGN
%token OR_ASSIGN
%token XOR_ASSIGN
%token AND_ASSIGN
%token AMPERSAND
%token BANG
%token QUESTION_MARK
%token UNDERSCORE
%token TILDE
%token DOT
%token DOTDOT
%token DOTDOTDOT
%token INCREMENT
%token DECREMENT
%token OPEN_PAREN
%token CLOSE_PAREN
%token OPEN_BRACKET
%token CLOSE_BRACKET
%token OPEN_BRACE
%token CLOSE_BRACE
%token SEMICOLON
%token COLON
%token COLON_COLON
%token COMMA
%token ARROW
%token ABSTRACT
%token AS
%token BASECLASS
%token BREAK
%token CATCH
%token CLASS
%token CONSTANT
%token CONTINUE
%token DATACLASS
%token DEFAULT
%token DEFER
%token DELETE
%token DO
%token ELSE
%token ENUM
%token EXTERN
%token FINALLY
%token FOR
%token FORWARD
%token FROM
%token FUN
%token IF
%token IMPLEMENTS
%token IMPORT
%token IN
%token INHERITS
%token INTERFACE
%token IS
%token MACRO
%token MUT
%token NEW
%token NOEXCEPT
%token OBJECT
%token OVERRIDE
%token PACKAGE
%token PRIVATE
%token PROTECTED
%token PUBLIC
%token PURE
%token REINTERPRET_CAST
%token RETURN
%token THROW
%token TRY
%token TRY_COMPILE
%token USING
%token VAL
%token VIRTUAL
%token WHEN
%token WHILE
%token WITH
%token YIELD

// Macro convenience tokens.
%token CLASS_MACRO
%token CLASS_MEMBER_MACRO
%token ENUM_MACRO
%token FUNCTION_MACRO
%token INTERFACE_MACRO
%token INTERFACE_MEMBER_MACRO
%token STATEMENT_LIST_MACRO
%token STATEMENT_MACRO
%token VALUE_MACRO

%precedence THEN
%precedence ELSE

%precedence QUALIFIED_TYPE

%precedence MUT

%precedence FUNCTION_VALUE

%precedence THROW
%nonassoc IN
%left OR
%left AND
%left EQUAL DIFFERENT
%left GREATER GREATER_OR_EQUAL LESS LESS_OR_EQUAL
%left PLUS MINUS
%left STAR DIVIDE INTEGER_DIVIDE MODULO
%left BITSHIFT_LEFT BITSHIFT_RIGHT
%left BITOR BITXOR AMPERSAND
%left QUESTION_MARK_COLON
%precedence UNARY
%precedence OPEN_BRACKET
%precedence OPEN_PAREN
%left DOT ARROW QUESTION_MARK_DOT QUESTION_MARK_ARROW

%%

relativeVarIdent: LOWER_CASE_IDENT
                | UPPER_CASE_IDENT COLON_COLON varIdent

varIdent: COLON_COLON relativeVarIdent
        | relativeVarIdent

relativeTypeIdent: UPPER_CASE_IDENT
                 | UPPER_CASE_IDENT COLON_COLON typeIdent

typeIdent: COLON_COLON relativeTypeIdent
         | relativeTypeIdent

unaryOp: AMPERSAND
       | STAR
       | BANG
       | TILDE
       | MINUS

constness: VAL | MUT

argConstness: constness
            | FORWARD

type: MUT unqualifiedType %prec QUALIFIED_TYPE
    | unqualifiedType %prec QUALIFIED_TYPE
    | OPEN_PAREN typeList CLOSE_PAREN ARROW type // function type

unqualifiedType: nonOptionType
               | unqualifiedType QUESTION_MARK
               | unqualifiedType MUT STAR
               | unqualifiedType STAR

constructibleType: typeIdent
                 | typeIdent BANG OPEN_BRACKET optionalTemplateValueList CLOSE_BRACKET

nonOptionType: constructibleType

optionalTemplateValueList: templateValueList
                         | %empty

templateValueList: templateValue
                 | templateValue COMMA templateValueList

templateValue: type
             | type DOTDOTDOT
             | value

varTypeList: varType COMMA varTypeList
           | varType

optionalVarTypeList: varTypeList
                   | %empty

lambda: OPEN_BRACE optionalVarTypeList ARROW stmtList CLOSE_BRACE

constant: INT
        | HEX
        | OCT
        | BINARY_NUMBER
        | FLOAT
        | STRING
        | RAWSTRING
        | CODESTRING
        | RAWCODESTRING

dotValue: varIdent
        | constant
        | OPEN_PAREN value CLOSE_PAREN
        | %empty

// Expressions that can appear before the parenthesis of a function call
functionValue: VALUE_MACRO
             | varIdent
             | funCall
             | funCall lambda
             | lambda
             | functionValue DOT functionValue
             | functionValue ARROW functionValue
             | functionValue QUESTION_MARK_DOT functionValue
             | functionValue QUESTION_MARK_ARROW functionValue
             | OPEN_PAREN value CLOSE_PAREN

rangeValue: dotValue DOTDOT dotValue

value: functionValue %prec FUNCTION_VALUE
     | constant
     | NEW type funParams
       // Arithmetic
     | value PLUS value
     | value MINUS value
     | value STAR value
     | value DIVIDE value
     | value INTEGER_DIVIDE value
     | value MODULO value
       // Boolean
     | value AND value
     | value OR value
       // Comparison
     | value GREATER_OR_EQUAL value
     | value GREATER value
     | value LESS_OR_EQUAL value
     | value LESS value
     | value DIFFERENT value
     | value EQUAL value
       // Bitwise
     | value BITSHIFT_LEFT value
     | value BITSHIFT_RIGHT value
     | value BITOR value
     | value BITXOR value
     | value AMPERSAND value
     | value QUESTION_MARK_COLON value
     | unaryOp value %prec UNARY
     | value OPEN_BRACKET value CLOSE_BRACKET
     | value OPEN_BRACKET rangeValue CLOSE_BRACKET
     | IF OPEN_PAREN optionalVarDecl value CLOSE_PAREN stmt %prec THEN
     | IF OPEN_PAREN optionalVarDecl value CLOSE_PAREN stmt ELSE stmt
     | whenBlockValue
     | tryBlock
     | THROW value
     | value IN rangeValue
     | value IN value
     | OPEN_BRACKET optionalValueList CLOSE_BRACKET
     | REINTERPRET_CAST BANG OPEN_BRACKET type CLOSE_BRACKET OPEN_PAREN value CLOSE_PAREN

varName: LOWER_CASE_IDENT
       | UNDERSCORE

varDeclConstness: constness
                | CONSTANT

varType: varDeclConstness varName
       | varDeclConstness varName COLON type

varDecl: varType
       | varTypeList ASSIGN value

arg: value
   | varName ASSIGN value

argList: arg
       | arg COMMA argList

optionalArgList: argList
               | %empty

typeList: type
        | type COMMA typeList

typeIdentList: typeIdent
             | typeIdent COMMA typeIdentList

funParams: OPEN_PAREN optionalArgList CLOSE_PAREN

funCall: functionValue funParams
       | varIdent BANG OPEN_BRACKET optionalTemplateValueList CLOSE_BRACKET funParams
       | constructibleType funParams // constructor

body: ASSIGN value SEMICOLON
      | stmtBlock

argDecl: argConstness varName COLON type DOTDOTDOT
       | argConstness varName ASSIGN value COLON type

argDeclList: argDecl
           | argDecl COMMA argDeclList

optionalArgDeclList: argDeclList
                   | %empty

optionalTypeDescription: COLON type
                       | %empty

optionalDots: DOTDOTDOT
            | %empty

templateDeclArg: UPPER_CASE_IDENT optionalDots
               | UPPER_CASE_IDENT optionalDots COLON type
               | LOWER_CASE_IDENT optionalDots COLON type

templateDeclArgList: templateDeclArg
                   | templateDeclArg COMMA templateDeclArgList

valueList: value
         | value COMMA valueList

optionalValueList: valueList
                 | %empty

optionalTemplateDeclWith: WITH valueList
                        | %empty

templateDecl: OPEN_BRACKET templateDeclArgList optionalTemplateDeclWith CLOSE_BRACKET
            | %empty

optionalFunctionQualifier: PURE
                         | EXTERN
                         | %empty

optionalNoExcept: NOEXCEPT
                | %empty

funDecl: FUNCTION_MACRO
       | optionalVisibilityDecl optionalFunctionQualifier FUN templateDecl LOWER_CASE_IDENT OPEN_PAREN optionalArgDeclList CLOSE_PAREN optionalTypeDescription optionalNoExcept body

memberFunDecl: ABSTRACT varDeclConstness FUN templateDecl LOWER_CASE_IDENT OPEN_PAREN optionalArgDeclList CLOSE_PAREN COLON type SEMICOLON
             | VIRTUAL varDeclConstness funDecl
             | OVERRIDE varDeclConstness funDecl
             | varDeclConstness funDecl

optionalFinally: FINALLY stmtBlock
               | %empty

catchTypeList: type
             | type BITOR catchTypeList

catchBlock: CATCH OPEN_PAREN varName COLON catchTypeList CLOSE_PAREN stmtBlock

catchList: catchBlock catchList
         | %empty

tryBlock: TRY stmtBlock catchList optionalFinally

usingStmt: USING typeIdent SEMICOLON
         | USING UPPER_CASE_IDENT ASSIGN typeIdent SEMICOLON

stmt: STATEMENT_MACRO
    | value SEMICOLON
    | varDecl SEMICOLON
    | assignment SEMICOLON
    | value INCREMENT SEMICOLON
    | value DECREMENT SEMICOLON
    | funDecl
    | RETURN value SEMICOLON
    | forLoop
    | whileLoop
    | stmtBlock
    | DEFER stmt
    | DELETE value SEMICOLON
    | usingStmt
    | TRY_COMPILE stmtBlock %prec THEN
    | TRY_COMPILE stmtBlock ELSE stmtBlock
    | BREAK SEMICOLON
    | CONTINUE SEMICOLON
    | YIELD value SEMICOLON
    | YIELD SEMICOLON

stmtList: STATEMENT_LIST_MACRO stmtList
        | stmt stmtList
        | %empty

stmtBlock: OPEN_BRACE stmtList CLOSE_BRACE

assignment: value ASSIGN value
          | value PLUS_ASSIGN value
          | value MINUS_ASSIGN value
          | value TIMES_ASSIGN value
          | value DIVIDE_ASSIGN value
          | value XOR_ASSIGN value
          | value OR_ASSIGN value
          | value AND_ASSIGN value

forLoop: FOR OPEN_PAREN varTypeList IN value CLOSE_PAREN stmt

optionalVarDecl: varTypeList ASSIGN value SEMICOLON
               | %empty

whileLoop: WHILE OPEN_PAREN optionalVarDecl value CLOSE_PAREN stmt
         | DO stmtBlock WHILE OPEN_PAREN value CLOSE_PAREN SEMICOLON


whenCaseValueList: whenCaseValue whenCaseValueList
                 | %empty

whenBlockValue: WHEN optionalWhenValue OPEN_BRACE whenCaseValueList CLOSE_BRACE

whenCaseValue: whenCondition ARROW stmt

optionalWhenValue: OPEN_PAREN value CLOSE_PAREN
                 | %empty

whenConditionElem: IN value
                 | IS type
                 | value

whenConditionList: whenConditionElem
                 | whenConditionElem COMMA whenConditionList

whenCondition: DEFAULT
             | whenConditionList



packageIdent: UPPER_CASE_IDENT
            | UPPER_CASE_IDENT COLON_COLON packageIdent


packageMember: varIdent AS varIdent
             | varIdent
             | typeIdent AS typeIdent
             | typeIdent


packageMemberList: packageMember
                 | packageMember COMMA packageMemberList

optionalPackageAlias: AS typeIdent
                    | %empty

importStmt: IMPORT packageIdent optionalPackageAlias SEMICOLON
          | FROM packageIdent IMPORT packageMemberList

classQualifier: DATACLASS
              | ABSTRACT BASECLASS
              | BASECLASS
              | CLASS
              | EXTERN CLASS
              | OBJECT

optionalInherits: INHERITS typeIdent
                | %empty

optionalImplements: IMPLEMENTS typeIdentList
                  | %empty

visibilityDecl: PRIVATE
              | PUBLIC
              | PROTECTED
              | PACKAGE

optionalVisibilityDecl: visibilityDecl
                      | %empty

classMemberDecl: memberFunDecl
               | varDecl
               | CLASS_MEMBER_MACRO
               | visibilityDecl COLON

classMemberDeclList: classMemberDecl classMemberDeclList
                   | %empty

interfaceMemberDeclList: INTERFACE_MEMBER_MACRO
                       | memberFunDecl interfaceMemberDeclList
                       | %empty

classDecl: CLASS_MACRO
         | optionalVisibilityDecl classQualifier templateDecl UPPER_CASE_IDENT optionalInherits optionalImplements OPEN_BRACE classMemberDeclList CLOSE_BRACE

enumValue: UPPER_CASE_IDENT OPEN_PAREN optionalArgList CLOSE_PAREN
         | UPPER_CASE_IDENT

enumValueList: enumValue
             | enumValue COMMA enumValueList

enumDecl: ENUM_MACRO
        | optionalVisibilityDecl classQualifier templateDecl UPPER_CASE_IDENT optionalInherits optionalImplements OPEN_BRACE enumValueList SEMICOLON classMemberDeclList CLOSE_BRACE

interfaceDecl: INTERFACE_MACRO
             | INTERFACE UPPER_CASE_IDENT optionalInherits OPEN_BRACE interfaceMemberDeclList CLOSE_BRACE

macroDecl: optionalVisibilityDecl MACRO MACRO_IDENT OPEN_PAREN optionalArgDeclList CLOSE_PAREN stmtBlock

topleveldecl: importStmt
            | varDecl
            | funDecl
            | classDecl
            | enumDecl
            | interfaceDecl
            | macroDecl
            | usingStmt

program: topleveldecl program
       | %empty




macroArg: value
        | stmtBlock

macroArgList: macroArg
            | macroArg COMMA macroArgList

optionalMacroArgList: macroArgList
                    | %empty

optionalFinalMacroParam: classDecl
                       | interfaceDecl
                       | stmtBlock
                       | SEMICOLON

optionalMacroCallParams: OPEN_PAREN optionalMacroArgList CLOSE_PAREN
                       | %empty

macroCall: MACRO_IDENT optionalMacroCallParams CLOSE_PAREN optionalFinalMacroParam

dummy: program
     | macroCall // just to check the grammar of a macro call

%%

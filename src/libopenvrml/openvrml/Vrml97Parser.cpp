/* $ANTLR 2.7.2: "Vrml97Parser.g" -> "Vrml97Parser.cpp"$ */
#line 118 "Vrml97Parser.g"

# include "Vrml97Parser.hpp"
ANTLR_RBRACE

#line 8 "Vrml97Parser.cpp"
#include "Vrml97Parser.hpp"
#include <antlr/NoViableAltException.hpp>
#include <antlr/SemanticException.hpp>
#include <antlr/ASTFactory.hpp>
#line 123 "Vrml97Parser.g"

# include <cctype>
# include <iostream>
# include <antlr/CommonToken.hpp>
# include <boost/lexical_cast.hpp>
# include "scope.h"
# include "script.h"
# include "private.h"

namespace {

bool isValidIdFirstChar(char);
bool isValidIdRestChars(char);
bool isWhitespaceChar(char);
bool isNewlineChar(char);
bool isHexDigit(char);

const int Vrml97Scanner::EOF_                   (antlr::Token::EOF_TYPE);

//
// The values and names for these constants *must* match those expressed in the
// file Vrml97TokenTypes.txt.
//
const int Vrml97Scanner::PERIOD                 (4);
const int Vrml97Scanner::LBRACKET               (5);
const int Vrml97Scanner::RBRACKET               (6);
const int Vrml97Scanner::LBRACE                 (7);
const int Vrml97Scanner::RBRACE                 (8);
const int Vrml97Scanner::ID                     (9);
const int Vrml97Scanner::INTEGER                (10);
const int Vrml97Scanner::HEX_INTEGER            (11);
const int Vrml97Scanner::REAL                   (12);
const int Vrml97Scanner::STRING                 (13);
const int Vrml97Scanner::KEYWORD_DEF            (14);
const int Vrml97Scanner::KEYWORD_EVENTIN        (15);
const int Vrml97Scanner::KEYWORD_EVENTOUT       (16);
const int Vrml97Scanner::KEYWORD_EXPOSEDFIELD   (17);
const int Vrml97Scanner::KEYWORD_EXTERNPROTO    (18);
const int Vrml97Scanner::KEYWORD_FALSE          (19);
const int Vrml97Scanner::KEYWORD_FIELD          (20);
const int Vrml97Scanner::KEYWORD_IS             (21);
const int Vrml97Scanner::KEYWORD_NULL           (22);
const int Vrml97Scanner::KEYWORD_PROTO          (23);
const int Vrml97Scanner::KEYWORD_ROUTE          (24);
const int Vrml97Scanner::KEYWORD_TO             (25);
const int Vrml97Scanner::KEYWORD_TRUE           (26);
const int Vrml97Scanner::KEYWORD_USE            (27);
const int Vrml97Scanner::FIELDTYPE_SFBOOL       (28);
const int Vrml97Scanner::FIELDTYPE_SFCOLOR      (29);
const int Vrml97Scanner::FIELDTYPE_SFFLOAT      (30);
const int Vrml97Scanner::FIELDTYPE_SFIMAGE      (31);
const int Vrml97Scanner::FIELDTYPE_SFINT32      (32);
const int Vrml97Scanner::FIELDTYPE_SFNODE       (33);
const int Vrml97Scanner::FIELDTYPE_SFROTATION   (34);
const int Vrml97Scanner::FIELDTYPE_SFSTRING     (35);
const int Vrml97Scanner::FIELDTYPE_SFTIME       (36);
const int Vrml97Scanner::FIELDTYPE_SFVEC2F      (37);
const int Vrml97Scanner::FIELDTYPE_SFVEC3F      (38);
const int Vrml97Scanner::FIELDTYPE_MFCOLOR      (39);
const int Vrml97Scanner::FIELDTYPE_MFFLOAT      (40);
const int Vrml97Scanner::FIELDTYPE_MFINT32      (41);
const int Vrml97Scanner::FIELDTYPE_MFNODE       (42);
const int Vrml97Scanner::FIELDTYPE_MFROTATION   (43);
const int Vrml97Scanner::FIELDTYPE_MFSTRING     (44);
const int Vrml97Scanner::FIELDTYPE_MFTIME       (45);
const int Vrml97Scanner::FIELDTYPE_MFVEC2F      (46);
const int Vrml97Scanner::FIELDTYPE_MFVEC3F      (47);

Vrml97Scanner::Vrml97Scanner(std::istream & in):
    in_(in),
    line_(1),
    col_(0),
    c_(' '),
    prev_char_('\0'),
    prev_token_type_(0),
    read_too_much_(false),
    expecting_field_type_(false)
{}

antlr::RefToken Vrml97Scanner::nextToken()
{
    using std::string;
    using antlr::RefToken;
    using antlr::CommonToken;

    RefToken token(new CommonToken);
    string token_string;

    if (this->read_too_much_) {
        this->read_too_much_ = false;
    } else {
        this->getNextChar();
    }

    while (isWhitespaceChar(this->c_) || this->c_ == '#') {
        if (this->c_ == '#') {
            while (!(isNewlineChar(this->c_) || this->c_ == EOF)) {
                this->getNextChar();
            }
        } else {
            getNextChar();
        }
    }

    if (this->c_ == EOF) {
        token->setType(EOF_);
    } else if (isValidIdFirstChar(this->c_)) {
        //
        // in an identifier or a keyword
        //
        token->setType(ID);

        while (isValidIdRestChars(this->c_)) {
            token_string += this->c_;
            getNextChar();
        }

        read_too_much_ = true;

        token->setText(token_string);

        if (expecting_field_type_) {
            this->identifyFieldType(*token);
            expecting_field_type_ = false;
        }
        this->identifyKeyword(*token);

    } else if (this->c_ == '.'
            || this->c_ == '+'
            || this->c_ == '-'
            || isdigit(this->c_)) {
        //
        // probably in a number
        //

        if (this->c_ == '+' || this->c_ == '-') {
            token_string += this->c_;
            this->getNextChar();
            read_too_much_ = true;
        }

        if (isdigit(this->c_)) {
            //
            // definitely in a number
            //
            token->setType(INTEGER);

            token_string += this->c_;

            getNextChar();

            if (this->prev_char_ == '0'
                    && (this->c_ == 'x' || this->c_ == 'X')) {
                //
                // in an integer expressed in hexadecimal
                //
                token->setType(HEX_INTEGER);

                token_string += this->c_;
                getNextChar();
                while (isHexDigit(this->c_)) {
                    token_string += this->c_;
                    getNextChar();
                }
            } else {
                while (isdigit(this->c_)) {
                    token_string += this->c_;
                    getNextChar();
                }
            }

            if (this->c_ == '.') {
                //
                // in a floating-point number
                //
                token->setType(REAL);

                token_string += this->c_;
                getNextChar();

                while (isdigit(this->c_)) {
                    token_string += this->c_;
                    getNextChar();
                }

                if (this->c_ == 'E' || this->c_ == 'e') {
                    //
                    // in an exponent
                    //
                    token_string += this->c_;
                    getNextChar();

                    if ((c_ == '+') || (c_ == '-') || isdigit(this->c_)) {
                        //
                        // exponent may be signed
                        //
                        token_string += this->c_;
                        getNextChar();

                        while (isdigit(this->c_)) {
                            token_string += this->c_;
                            getNextChar();
                        }
                    }
                }
            } else if (this->c_ == 'E' || this->c_ == 'e') {
                //
                // in an exponent
                //
                token->setType(REAL);

                token_string += this->c_;
                getNextChar();

                if (this->c_ == '+' || this->c_ == '-' || isdigit(this->c_)) {
                    //
                    // exponent may be signed
                    //
                    token_string += this->c_;
                    getNextChar();

                    while (isdigit(this->c_)) {
                        token_string += this->c_;
                        getNextChar();
                    }
                }
            }

            this->read_too_much_ = true;

        } else if (this->c_ == '.') {
            //
            // in a floating-point number or a lone full-stop (as in a ROUTE)
            //

            token_string += this->c_;
            getNextChar();

            if (isdigit(this->c_)) {

                token->setType(REAL);

                while (isdigit(this->c_)) {
                    token_string += this->c_;
                    getNextChar();
                }

                if ((this->c_ == 'E') || (this->c_ == 'e')) {
                    //
                    // in an exponent
                    //
                    token_string += this->c_;
                    getNextChar();

                    if ((c_ == '+') || (c_ == '-') || isdigit(this->c_)) {
                        //
                        // exponent may be signed
                        //
                        token_string += this->c_;
                        getNextChar();

                        while (isdigit(this->c_)) {
                            token_string += this->c_;
                            getNextChar();
                        }
                    }
                }
            } else {
                token->setType(PERIOD);
            }

            this->read_too_much_ = true;
        }

        token->setText(token_string);

    } else if (this->c_ == '"') {
        //
        // in a string
        //
        token->setType(STRING);

        token_string += this->c_;
        this->getNextChar();

        char prev_char('\0');
        while (this->c_ != '"' || prev_char == '\\') {
            token_string += this->c_;
            prev_char = this->c_;
            this->getNextChar();
        }
        token_string += this->c_; // add the closing quote

        token->setText(token_string);

    } else {
        //
        // terminal symbol or unidentified character
        //
        token_string += this->c_;

        token->setText(token_string);

        this->identifyTerminalSymbol(*token);
    }

    token->setLine(int(this->line_));
    token->setColumn(int(this->col_));
    this->prev_token_type_ = token->getType();

    return token;
}

inline void Vrml97Scanner::getNextChar()
{
    this->prev_char_ = this->c_;
    this->c_ = this->in_.get();
    ++this->col_; // Increment the column count;

    //
    // Increment the line count (and reset the column count to zero) if the
    // current character is a newline character EXCEPT if the current character
    // is a linefeed AND the previous character is a carriage return.
    //
    if (isNewlineChar(this->c_)) {
        if (!((this->c_ == 0x0a) && (this->prev_char_ == 0x0d))) {
            ++this->line_;
            this->col_ = 0;
        }
    }
}

inline void Vrml97Scanner::identifyKeyword(antlr::Token & token)
{
    std::string const token_text(token.getText());
    if      (token_text == "DEF")          { token.setType(KEYWORD_DEF); }
    else if (token_text == "eventIn")      { expecting_field_type_ = true;
                                             token.setType(KEYWORD_EVENTIN); }
    else if (token_text == "eventOut")     { expecting_field_type_ = true;
                                             token.setType(KEYWORD_EVENTOUT); }
    else if (token_text == "exposedField") { expecting_field_type_ = true;
                                             token.setType(KEYWORD_EXPOSEDFIELD); }
    else if (token_text == "EXTERNPROTO")  { token.setType(KEYWORD_EXTERNPROTO); }
    else if (token_text == "FALSE")        { token.setType(KEYWORD_FALSE); }
    else if (token_text == "field")        { expecting_field_type_ = true;
                                             token.setType(KEYWORD_FIELD); }
    else if (token_text == "IS")           { token.setType(KEYWORD_IS); }
    else if (token_text == "NULL")         { token.setType(KEYWORD_NULL); }
    else if (token_text == "PROTO")        { token.setType(KEYWORD_PROTO); }
    else if (token_text == "ROUTE")        { token.setType(KEYWORD_ROUTE); }
    else if (token_text == "TO")           { token.setType(KEYWORD_TO); }
    else if (token_text == "TRUE")         { token.setType(KEYWORD_TRUE); }
    else if (token_text == "USE")          { token.setType(KEYWORD_USE); }
}

inline void Vrml97Scanner::identifyFieldType(antlr::Token & token)
{
    assert(this->expecting_field_type_);
    std::string const token_text(token.getText());
    if      (token_text == "SFBool")     { token.setType(FIELDTYPE_SFBOOL); }
    else if (token_text == "SFColor")    { token.setType(FIELDTYPE_SFCOLOR); }
    else if (token_text == "SFFloat")    { token.setType(FIELDTYPE_SFFLOAT); }
    else if (token_text == "SFImage")    { token.setType(FIELDTYPE_SFIMAGE); }
    else if (token_text == "SFInt32")    { token.setType(FIELDTYPE_SFINT32); }
    else if (token_text == "SFNode")     { token.setType(FIELDTYPE_SFNODE); }
    else if (token_text == "SFRotation") { token.setType(FIELDTYPE_SFROTATION); }
    else if (token_text == "SFString")   { token.setType(FIELDTYPE_SFSTRING); }
    else if (token_text == "SFTime")     { token.setType(FIELDTYPE_SFTIME); }
    else if (token_text == "SFVec2f")    { token.setType(FIELDTYPE_SFVEC2F); }
    else if (token_text == "SFVec3f")    { token.setType(FIELDTYPE_SFVEC3F); }
    else if (token_text == "MFColor")    { token.setType(FIELDTYPE_MFCOLOR); }
    else if (token_text == "MFFloat")    { token.setType(FIELDTYPE_MFFLOAT); }
    else if (token_text == "MFInt32")    { token.setType(FIELDTYPE_MFINT32); }
    else if (token_text == "MFNode")     { token.setType(FIELDTYPE_MFNODE); }
    else if (token_text == "MFRotation") { token.setType(FIELDTYPE_MFROTATION); }
    else if (token_text == "MFString")   { token.setType(FIELDTYPE_MFSTRING); }
    else if (token_text == "MFTime")     { token.setType(FIELDTYPE_MFTIME); }
    else if (token_text == "MFVec2f")    { token.setType(FIELDTYPE_MFVEC2F); }
    else if (token_text == "MFVec3f")    { token.setType(FIELDTYPE_MFVEC3F); }
}

inline void Vrml97Scanner::identifyTerminalSymbol(antlr::Token & token)
{
    std::string const token_text(token.getText());
    if      (token_text == "[") { token.setType(LBRACKET); }
    else if (token_text == "]") { token.setType(RBRACKET); }
    else if (token_text == "{") { token.setType(LBRACE); }
    else if (token_text == "}") { token.setType(RBRACE); }
}

inline bool isValidIdFirstChar(const char c)
{
    if (((c >= 0x30) && (c <= 0x39))
            || (c == 0x2b)
            || (c == 0x2d)
            || !isValidIdRestChars(c)) {
        return false;
    }
    return true;
}

inline bool isValidIdRestChars(const char c)
{
    if (       c <= 0x20
            || c == 0x22
            || c == 0x23
            || c == 0x27
            || c == 0x2c
            || c == 0x2e
            || c == 0x5b
            || c == 0x5c
            || c == 0x5d
            || c == 0x7b
            || c == 0x7d
            || c == 0x7f) {
        return false;
    }
    return true;
}

inline bool isWhitespaceChar(const char c)
{
    if (       c == 0x0d      // carriage return
            || c == 0x0a      // linefeed
            || c == 0x20      // space
            || c == 0x09      // tab
            || c == 0x2c) {   // comma
        return true;
    }
    return false;
}

inline bool isNewlineChar(const char c)
{
    return ((c == 0x0a) || (c == 0x0d));
}

inline bool isHexDigit(const char c)
{
    if (          isdigit(c)
            || c == 'A' || c == 'a'
            || c == 'B' || c == 'b'
            || c == 'C' || c == 'c'
            || c == 'D' || c == 'd'
            || c == 'E' || c == 'e'
            || c == 'F' || c == 'f') {
        return true;
    }
    return false;
}

const openvrml::node_interface script_node_interfaces[] = {
    openvrml::node_interface(openvrml::node_interface::exposedfield_id,
                             openvrml::field_value::mfstring_id,
                             "url"),
    openvrml::node_interface(openvrml::node_interface::field_id,
                             openvrml::field_value::sfbool_id,
                             "directOutput"),
    openvrml::node_interface(openvrml::node_interface::field_id,
                             openvrml::field_value::sfbool_id,
                             "mustEvaluate")
};

const openvrml::node_interface_set
    script_node_interface_set(script_node_interfaces,
                              script_node_interfaces + 3);

} // namespace

namespace openvrml ANTLR_LBRACE


#line 486 "Vrml97Parser.cpp"
#line 1 "Vrml97Parser.g"
#line 488 "Vrml97Parser.cpp"
Vrml97Parser::Vrml97Parser(ANTLR_USE_NAMESPACE(antlr)TokenBuffer& tokenBuf, int k)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(tokenBuf,k)
{
}

Vrml97Parser::Vrml97Parser(ANTLR_USE_NAMESPACE(antlr)TokenBuffer& tokenBuf)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(tokenBuf,1)
{
}

Vrml97Parser::Vrml97Parser(ANTLR_USE_NAMESPACE(antlr)TokenStream& lexer, int k)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(lexer,k)
{
}

Vrml97Parser::Vrml97Parser(ANTLR_USE_NAMESPACE(antlr)TokenStream& lexer)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(lexer,1)
{
}

Vrml97Parser::Vrml97Parser(const ANTLR_USE_NAMESPACE(antlr)ParserSharedInputState& state)
: ANTLR_USE_NAMESPACE(antlr)LLkParser(state,1)
{
}

void Vrml97Parser::vrmlScene(
	openvrml::browser & browser,
          std::vector<node_ptr> & nodes
) {
#line 617 "Vrml97Parser.g"
	
	const scope_ptr scope(new Vrml97RootScope(browser, this->uri));
	
#line 522 "Vrml97Parser.cpp"
	
	{ // ( ... )*
	for (;;) {
		if ((_tokenSet_0.member(LA(1)))) {
			statement(browser, nodes, scope);
		}
		else {
			goto _loop3;
		}
		
	}
	_loop3:;
	} // ( ... )*
}

void Vrml97Parser::statement(
	openvrml::browser & browser,
          std::vector<node_ptr> & nodes,
          const scope_ptr & scope
) {
#line 626 "Vrml97Parser.g"
	
	node_ptr node;
	node_type_ptr nodeType;
	
#line 548 "Vrml97Parser.cpp"
	
	switch ( LA(1)) {
	case ID:
	case KEYWORD_DEF:
	case KEYWORD_USE:
	{
		node=nodeStatement(browser, scope);
#line 634 "Vrml97Parser.g"
		
		//
		// If we are unable to parse a node, node will be null.
		//
		if (node) { nodes.push_back(node); }
		
#line 563 "Vrml97Parser.cpp"
		break;
	}
	case KEYWORD_EXTERNPROTO:
	case KEYWORD_PROTO:
	{
		protoStatement(browser, scope);
		break;
	}
	case KEYWORD_ROUTE:
	{
		routeStatement(*scope);
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
}

node_ptr  Vrml97Parser::nodeStatement(
	openvrml::browser & browser, const scope_ptr & scope
) {
#line 644 "Vrml97Parser.g"
	node_ptr n;
#line 589 "Vrml97Parser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  id0 = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  id1 = ANTLR_USE_NAMESPACE(antlr)nullToken;
	
	switch ( LA(1)) {
	case KEYWORD_DEF:
	{
		match(KEYWORD_DEF);
		id0 = LT(1);
		match(ID);
		n=node(browser, scope, id0->getText());
		break;
	}
	case KEYWORD_USE:
	{
		match(KEYWORD_USE);
		id1 = LT(1);
		match(ID);
#line 649 "Vrml97Parser.g"
		
		using antlr::SemanticException;
		assert(scope);
		n.reset(scope->find_node(id1->getText()));
		if (!n) {
		throw SemanticException("Node \"" + id1->getText()
		+ "\" has not been defined in this "
		"scope.",
		this->uri,
		id1->getLine(),
		id1->getColumn());
		}
		
#line 621 "Vrml97Parser.cpp"
		break;
	}
	case ID:
	{
		n=node(browser, scope, std::string());
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return n;
}

void Vrml97Parser::protoStatement(
	openvrml::browser & browser, const scope_ptr & scope
) {
	
	switch ( LA(1)) {
	case KEYWORD_EXTERNPROTO:
	{
		externproto(browser, scope);
		break;
	}
	case KEYWORD_PROTO:
	{
		proto(browser, scope);
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
}

void Vrml97Parser::routeStatement(
	const openvrml::scope & scope
) {
	ANTLR_USE_NAMESPACE(antlr)RefToken  from_node_id = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  eventout_id = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  to_node_id = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  eventin_id = ANTLR_USE_NAMESPACE(antlr)nullToken;
	
	match(KEYWORD_ROUTE);
	from_node_id = LT(1);
	match(ID);
	match(PERIOD);
	eventout_id = LT(1);
	match(ID);
	match(KEYWORD_TO);
	to_node_id = LT(1);
	match(ID);
	match(PERIOD);
	eventin_id = LT(1);
	match(ID);
#line 971 "Vrml97Parser.g"
	
	using std::runtime_error;
	using antlr::SemanticException;
	
	openvrml::node * const from_node =
	scope.find_node(from_node_id->getText());
	if (!from_node) {
	throw SemanticException("Node \"" + from_node_id->getText()
	+ "\" has not been defined in this "
	"scope.",
	this->uri,
	from_node_id->getLine(),
	from_node_id->getColumn());
	}
	
	openvrml::node * const to_node =
	scope.find_node(to_node_id->getText());
	if (!to_node) {
	throw SemanticException("Node \"" + to_node_id->getText()
	+ "\" has not been defined in this "
	"scope.",
	this->uri,
	to_node_id->getLine(),
	to_node_id->getColumn());
	}
	
	try {
	add_route(*from_node, eventout_id->getText(),
	*to_node, eventin_id->getText());
	} catch (runtime_error & ex) {
	throw SemanticException(ex.what(),
	this->uri,
	from_node_id->getLine(),
	from_node_id->getColumn());
	}
	
#line 716 "Vrml97Parser.cpp"
}

node_ptr  Vrml97Parser::node(
	openvrml::browser & browser,
     const scope_ptr & scope,
     const std::string & nodeId
) {
#line 1092 "Vrml97Parser.g"
	node_ptr n;
#line 726 "Vrml97Parser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  scriptId = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  nodeTypeId = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 1092 "Vrml97Parser.g"
	
	using antlr::SemanticException;
	
	initial_value_map initial_values;
	node_interface_set interfaces;
	node_type_ptr nodeType;
	
#line 737 "Vrml97Parser.cpp"
	
	try {      // for error handling
		if (((LA(1) == ID))&&( !LT(1)->getText().compare("Script") )) {
			scriptId = LT(1);
			match(ID);
			match(LBRACE);
			{ // ( ... )*
			for (;;) {
				switch ( LA(1)) {
				case ID:
				case KEYWORD_EXTERNPROTO:
				case KEYWORD_PROTO:
				case KEYWORD_ROUTE:
				{
					nodeBodyElement(browser,
                            scope,
                            script_node_interface_set,
                            initial_values);
					break;
				}
				case KEYWORD_EVENTIN:
				case KEYWORD_EVENTOUT:
				case KEYWORD_FIELD:
				{
					scriptInterfaceDeclaration(browser,
                                         scope,
                                         interfaces,
                                         initial_values);
					break;
				}
				default:
				{
					goto _loop32;
				}
				}
			}
			_loop32:;
			} // ( ... )*
			match(RBRACE);
#line 1113 "Vrml97Parser.g"
			
			n.reset(new script_node(browser.script_node_class_,
			scope,
			interfaces,
			initial_values));
			if (!nodeId.empty()) { n->id(nodeId); }
			
#line 785 "Vrml97Parser.cpp"
		}
		else if ((LA(1) == ID)) {
			nodeTypeId = LT(1);
			match(ID);
#line 1120 "Vrml97Parser.g"
			
			nodeType = scope->find_type(nodeTypeId->getText());
			if (!nodeType) {
			throw SemanticException("Unknown node type \""
			+ nodeTypeId->getText() + "\".",
			this->uri,
			nodeTypeId->getLine(),
			nodeTypeId->getColumn());
			}
			
			
#line 802 "Vrml97Parser.cpp"
			match(LBRACE);
			{ // ( ... )*
			for (;;) {
				if ((_tokenSet_1.member(LA(1)))) {
					nodeBodyElement(browser,
                                  scope,
                                  nodeType->interfaces(),
                                  initial_values);
				}
				else {
					goto _loop34;
				}
				
			}
			_loop34:;
			} // ( ... )*
			match(RBRACE);
#line 1134 "Vrml97Parser.g"
			
			n = node_ptr(nodeType->create_node(scope, initial_values));
			
			if (!nodeId.empty()) { n->id(nodeId); }
			
#line 826 "Vrml97Parser.cpp"
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
	}
	catch (std::invalid_argument & ex) {
#line 1141 "Vrml97Parser.g"
		
		throw SemanticException(ex.what());
		
#line 838 "Vrml97Parser.cpp"
	}
	catch (unsupported_interface & ex) {
#line 1144 "Vrml97Parser.g"
		
		throw SemanticException(ex.what());
		
#line 845 "Vrml97Parser.cpp"
	}
	catch (std::bad_cast & ex) {
#line 1147 "Vrml97Parser.g"
		
		throw SemanticException("Incorrect value type for field or "
		"exposedField.");
		
#line 853 "Vrml97Parser.cpp"
	}
	return n;
}

void Vrml97Parser::externproto(
	openvrml::browser & browser, const openvrml::scope_ptr & scope
) {
	ANTLR_USE_NAMESPACE(antlr)RefToken  id = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 886 "Vrml97Parser.g"
	
	openvrml::node_interface_set interfaces;
	openvrml::mfstring urlList;
	openvrml::node_type_ptr nodeType;
	
#line 868 "Vrml97Parser.cpp"
	
	match(KEYWORD_EXTERNPROTO);
	id = LT(1);
	match(ID);
	match(LBRACKET);
	{ // ( ... )*
	for (;;) {
		if ((_tokenSet_2.member(LA(1)))) {
			externInterfaceDeclaration(interfaces);
		}
		else {
			goto _loop22;
		}
		
	}
	_loop22:;
	} // ( ... )*
	match(RBRACKET);
	urlList=externprotoUrlList();
#line 895 "Vrml97Parser.g"
	
	for (size_t i = 0; i < urlList.value.size(); ++i) {
		browser::node_class_map_t::const_iterator pos =
	browser.node_class_map.find(urlList.value[i]);
	if (pos != browser.node_class_map.end()) {
	nodeType = pos->second->create_type(id->getText(),
	interfaces);
	break;
	}
	}
	//
	// If we weren't able to create a node_type, that means that we
	// don't already have a node_class for the node. Currently we only
	// support referring to existing node_classes with EXTERNPROTO;
	// adding new node_classes via EXTERNPROTO is not supported. In
	// practice, this means that the ordinary way of using EXTERNPROTOs
	// in VRML worlds will fail.
	//
	if (nodeType) {
	if (!scope->add_type(nodeType)) {
	using antlr::SemanticException;
	throw SemanticException("Node type \"" + nodeType->id
	+ "\" has already been defined in "
	" this scope.",
	this->uri,
	id->getLine(),
	id->getColumn());
	}
	}
	
#line 919 "Vrml97Parser.cpp"
}

void Vrml97Parser::proto(
	openvrml::browser & browser, const scope_ptr & scope
) {
	ANTLR_USE_NAMESPACE(antlr)RefToken  id = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 675 "Vrml97Parser.g"
	
	using std::vector;
	
	node_interface_set interfaces;
	proto_node_class::default_value_map_t default_value_map;
	vector<node_ptr> impl_nodes;
	proto_node_class::is_map_t is_map;
	proto_node_class::routes_t routes;
	
#line 936 "Vrml97Parser.cpp"
	
	match(KEYWORD_PROTO);
	id = LT(1);
	match(ID);
#line 686 "Vrml97Parser.g"
	
	scope_ptr proto_scope(new openvrml::scope(id->getText(), scope));
	
#line 945 "Vrml97Parser.cpp"
	match(LBRACKET);
	{ // ( ... )*
	for (;;) {
		if ((_tokenSet_2.member(LA(1)))) {
			protoInterfaceDeclaration(browser,
                                              scope,
                                              id->getText(),
                                              interfaces,
                                              default_value_map);
		}
		else {
			goto _loop9;
		}
		
	}
	_loop9:;
	} // ( ... )*
	match(RBRACKET);
	match(LBRACE);
	protoBody(browser,
                         proto_scope,
                         interfaces,
                         impl_nodes,
                         is_map,
                         routes);
	match(RBRACE);
#line 698 "Vrml97Parser.g"
	
	node_class_ptr node_class(new proto_node_class(browser,
	interfaces,
	default_value_map,
	impl_nodes,
	is_map,
	routes));
	//
	// Add the new node_class (prototype definition) to the browser's
	// node_class_map.
	//
	// First, construct the id for the node implementation.
	//
	std::string impl_id;
	do {
	impl_id = '#' + proto_scope->id + impl_id;
	} while ((proto_scope = proto_scope->parent));
	impl_id = scope->id + impl_id;
	browser.node_class_map.insert(make_pair(impl_id, node_class));
	
	//
	// PROTOs implicitly introduce a new node type as well.
	//
	const node_type_ptr node_type =
	node_class->create_type(id->getText(), interfaces);
	assert(node_type);
	assert(scope);
	if (!scope->add_type(node_type)) {
	using antlr::SemanticException;
	throw SemanticException("Node type \"" + node_type->id
	+ "\" has already been defined in "
	"this scope.",
	this->uri,
	id->getLine(),
	id->getColumn());
	}
	
#line 1010 "Vrml97Parser.cpp"
}

void Vrml97Parser::protoInterfaceDeclaration(
	
    openvrml::browser & browser,
    const scope_ptr & outer_scope,
    const std::string & proto_id,
    node_interface_set & interfaces,
    proto_node_class::default_value_map_t & default_value_map
) {
	ANTLR_USE_NAMESPACE(antlr)RefToken  id0 = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  id1 = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 737 "Vrml97Parser.g"
	
	using std::invalid_argument;
	using antlr::SemanticException;
	
	node_interface::type_id it;
	field_value::type_id ft;
	field_value_ptr fv;
	
#line 1032 "Vrml97Parser.cpp"
	
	switch ( LA(1)) {
	case KEYWORD_EVENTIN:
	case KEYWORD_EVENTOUT:
	{
		it=eventInterfaceType();
		ft=fieldType();
		id0 = LT(1);
		match(ID);
#line 752 "Vrml97Parser.g"
		
		try {
		add_interface(interfaces,
		node_interface(it, ft, id0->getText()));
		} catch (invalid_argument & ex) {
		throw SemanticException(ex.what(),
		this->uri,
		id0->getLine(),
		id0->getColumn());
		}
		
#line 1054 "Vrml97Parser.cpp"
		break;
	}
	case KEYWORD_EXPOSEDFIELD:
	case KEYWORD_FIELD:
	{
		it=fieldInterfaceType();
		ft=fieldType();
		id1 = LT(1);
		match(ID);
#line 764 "Vrml97Parser.g"
		
		//
		// The field value declaration should have access to the node
		// types in the outer scope.
		//
		const scope_ptr field_decl_scope(
		new scope(proto_id + '.' + id1->getText(), outer_scope));
		
#line 1073 "Vrml97Parser.cpp"
		fv=fieldValue(browser, field_decl_scope, ft);
#line 772 "Vrml97Parser.g"
		
		assert(fv);
		try {
		add_interface(interfaces,
		node_interface(it, ft, id1->getText()));
		} catch (invalid_argument & ex) {
		throw SemanticException(ex.what(),
		this->uri,
		id0->getLine(),
		id0->getColumn());
		}
		const bool succeeded =
		default_value_map.insert(make_pair(id1->getText(), fv)).second;
		assert(succeeded);
		
#line 1091 "Vrml97Parser.cpp"
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
}

void Vrml97Parser::protoBody(
	openvrml::browser & browser,
          const scope_ptr & scope,
          const node_interface_set & interfaces,
          std::vector<node_ptr> & impl_nodes,
          proto_node_class::is_map_t & is_map,
          proto_node_class::routes_t & routes
) {
#line 801 "Vrml97Parser.g"
	
	assert(scope);
	assert(impl_nodes.empty());
	assert(is_map.empty());
	assert(routes.empty());
	
	node_ptr n;
	
#line 1118 "Vrml97Parser.cpp"
	
	{ // ( ... )*
	for (;;) {
		if ((LA(1) == KEYWORD_EXTERNPROTO || LA(1) == KEYWORD_PROTO)) {
			protoStatement(browser, scope);
		}
		else {
			goto _loop15;
		}
		
	}
	_loop15:;
	} // ( ... )*
	n=protoNodeStatement(browser, scope, interfaces, is_map, routes);
#line 818 "Vrml97Parser.g"
	
	assert(n);
	impl_nodes.push_back(n);
	
#line 1138 "Vrml97Parser.cpp"
	{ // ( ... )*
	for (;;) {
		if ((_tokenSet_0.member(LA(1)))) {
			protoBodyStatement(browser,
                            scope,
                            interfaces,
                            impl_nodes,
                            is_map,
                            routes);
		}
		else {
			goto _loop17;
		}
		
	}
	_loop17:;
	} // ( ... )*
}

node_interface::type_id  Vrml97Parser::eventInterfaceType() {
#line 789 "Vrml97Parser.g"
	node_interface::type_id it;
#line 1161 "Vrml97Parser.cpp"
	
	switch ( LA(1)) {
	case KEYWORD_EVENTIN:
	{
		match(KEYWORD_EVENTIN);
#line 791 "Vrml97Parser.g"
		it = node_interface::eventin_id;
#line 1169 "Vrml97Parser.cpp"
		break;
	}
	case KEYWORD_EVENTOUT:
	{
		match(KEYWORD_EVENTOUT);
#line 792 "Vrml97Parser.g"
		it = node_interface::eventout_id;
#line 1177 "Vrml97Parser.cpp"
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return it;
}

openvrml::field_value::type_id  Vrml97Parser::fieldType() {
#line 1495 "Vrml97Parser.g"
	openvrml::field_value::type_id ft =
         openvrml::field_value::invalid_type_id;
#line 1192 "Vrml97Parser.cpp"
#line 1495 "Vrml97Parser.g"
	
	using openvrml::field_value;
	
#line 1197 "Vrml97Parser.cpp"
	
	switch ( LA(1)) {
	case FIELDTYPE_MFCOLOR:
	{
		match(FIELDTYPE_MFCOLOR);
#line 1502 "Vrml97Parser.g"
		ft = field_value::mfcolor_id;
#line 1205 "Vrml97Parser.cpp"
		break;
	}
	case FIELDTYPE_MFFLOAT:
	{
		match(FIELDTYPE_MFFLOAT);
#line 1503 "Vrml97Parser.g"
		ft = field_value::mffloat_id;
#line 1213 "Vrml97Parser.cpp"
		break;
	}
	case FIELDTYPE_MFINT32:
	{
		match(FIELDTYPE_MFINT32);
#line 1504 "Vrml97Parser.g"
		ft = field_value::mfint32_id;
#line 1221 "Vrml97Parser.cpp"
		break;
	}
	case FIELDTYPE_MFNODE:
	{
		match(FIELDTYPE_MFNODE);
#line 1505 "Vrml97Parser.g"
		ft = field_value::mfnode_id;
#line 1229 "Vrml97Parser.cpp"
		break;
	}
	case FIELDTYPE_MFROTATION:
	{
		match(FIELDTYPE_MFROTATION);
#line 1506 "Vrml97Parser.g"
		ft = field_value::mfrotation_id;
#line 1237 "Vrml97Parser.cpp"
		break;
	}
	case FIELDTYPE_MFSTRING:
	{
		match(FIELDTYPE_MFSTRING);
#line 1507 "Vrml97Parser.g"
		ft = field_value::mfstring_id;
#line 1245 "Vrml97Parser.cpp"
		break;
	}
	case FIELDTYPE_MFTIME:
	{
		match(FIELDTYPE_MFTIME);
#line 1508 "Vrml97Parser.g"
		ft = field_value::mftime_id;
#line 1253 "Vrml97Parser.cpp"
		break;
	}
	case FIELDTYPE_MFVEC2F:
	{
		match(FIELDTYPE_MFVEC2F);
#line 1509 "Vrml97Parser.g"
		ft = field_value::mfvec2f_id;
#line 1261 "Vrml97Parser.cpp"
		break;
	}
	case FIELDTYPE_MFVEC3F:
	{
		match(FIELDTYPE_MFVEC3F);
#line 1510 "Vrml97Parser.g"
		ft = field_value::mfvec3f_id;
#line 1269 "Vrml97Parser.cpp"
		break;
	}
	case FIELDTYPE_SFBOOL:
	{
		match(FIELDTYPE_SFBOOL);
#line 1511 "Vrml97Parser.g"
		ft = field_value::sfbool_id;
#line 1277 "Vrml97Parser.cpp"
		break;
	}
	case FIELDTYPE_SFCOLOR:
	{
		match(FIELDTYPE_SFCOLOR);
#line 1512 "Vrml97Parser.g"
		ft = field_value::sfcolor_id;
#line 1285 "Vrml97Parser.cpp"
		break;
	}
	case FIELDTYPE_SFFLOAT:
	{
		match(FIELDTYPE_SFFLOAT);
#line 1513 "Vrml97Parser.g"
		ft = field_value::sffloat_id;
#line 1293 "Vrml97Parser.cpp"
		break;
	}
	case FIELDTYPE_SFIMAGE:
	{
		match(FIELDTYPE_SFIMAGE);
#line 1514 "Vrml97Parser.g"
		ft = field_value::sfimage_id;
#line 1301 "Vrml97Parser.cpp"
		break;
	}
	case FIELDTYPE_SFINT32:
	{
		match(FIELDTYPE_SFINT32);
#line 1515 "Vrml97Parser.g"
		ft = field_value::sfint32_id;
#line 1309 "Vrml97Parser.cpp"
		break;
	}
	case FIELDTYPE_SFNODE:
	{
		match(FIELDTYPE_SFNODE);
#line 1516 "Vrml97Parser.g"
		ft = field_value::sfnode_id;
#line 1317 "Vrml97Parser.cpp"
		break;
	}
	case FIELDTYPE_SFROTATION:
	{
		match(FIELDTYPE_SFROTATION);
#line 1517 "Vrml97Parser.g"
		ft = field_value::sfrotation_id;
#line 1325 "Vrml97Parser.cpp"
		break;
	}
	case FIELDTYPE_SFSTRING:
	{
		match(FIELDTYPE_SFSTRING);
#line 1518 "Vrml97Parser.g"
		ft = field_value::sfstring_id;
#line 1333 "Vrml97Parser.cpp"
		break;
	}
	case FIELDTYPE_SFTIME:
	{
		match(FIELDTYPE_SFTIME);
#line 1519 "Vrml97Parser.g"
		ft = field_value::sftime_id;
#line 1341 "Vrml97Parser.cpp"
		break;
	}
	case FIELDTYPE_SFVEC2F:
	{
		match(FIELDTYPE_SFVEC2F);
#line 1520 "Vrml97Parser.g"
		ft = field_value::sfvec2f_id;
#line 1349 "Vrml97Parser.cpp"
		break;
	}
	case FIELDTYPE_SFVEC3F:
	{
		match(FIELDTYPE_SFVEC3F);
#line 1521 "Vrml97Parser.g"
		ft = field_value::sfvec3f_id;
#line 1357 "Vrml97Parser.cpp"
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return ft;
}

node_interface::type_id  Vrml97Parser::fieldInterfaceType() {
#line 795 "Vrml97Parser.g"
	node_interface::type_id it;
#line 1371 "Vrml97Parser.cpp"
	
	switch ( LA(1)) {
	case KEYWORD_FIELD:
	{
		match(KEYWORD_FIELD);
#line 797 "Vrml97Parser.g"
		it = node_interface::field_id;
#line 1379 "Vrml97Parser.cpp"
		break;
	}
	case KEYWORD_EXPOSEDFIELD:
	{
		match(KEYWORD_EXPOSEDFIELD);
#line 798 "Vrml97Parser.g"
		it = node_interface::exposedfield_id;
#line 1387 "Vrml97Parser.cpp"
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return it;
}

openvrml::field_value_ptr  Vrml97Parser::fieldValue(
	openvrml::browser & browser,
           const openvrml::scope_ptr & scope,
           openvrml::field_value::type_id ft
) {
#line 1524 "Vrml97Parser.g"
	openvrml::field_value_ptr fv;
#line 1405 "Vrml97Parser.cpp"
#line 1524 "Vrml97Parser.g"
	
	using openvrml::field_value;
	
#line 1410 "Vrml97Parser.cpp"
	
	if (((_tokenSet_3.member(LA(1))))&&( (ft == field_value::sfnode_id) || (ft == field_value::mfnode_id) )) {
		fv=nodeFieldValue(browser, scope, ft);
	}
	else if ((_tokenSet_4.member(LA(1)))) {
		fv=nonNodeFieldValue(ft);
	}
	else {
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	
	return fv;
}

node_ptr  Vrml97Parser::protoNodeStatement(
	openvrml::browser & browser,
                   const openvrml::scope_ptr & scope,
                   const node_interface_set & proto_interfaces,
                   proto_node_class::is_map_t & is_map,
                   proto_node_class::routes_t & routes
) {
#line 851 "Vrml97Parser.g"
	node_ptr n;
#line 1434 "Vrml97Parser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  id0 = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  id1 = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 851 "Vrml97Parser.g"
	
	using antlr::SemanticException;
	
#line 1441 "Vrml97Parser.cpp"
	
	switch ( LA(1)) {
	case KEYWORD_DEF:
	{
		match(KEYWORD_DEF);
		id0 = LT(1);
		match(ID);
		n=protoNode(browser,
                                       scope,
                                       proto_interfaces,
                                       is_map,
                                       routes,
                                       id0->getText());
		break;
	}
	case KEYWORD_USE:
	{
		match(KEYWORD_USE);
		id1 = LT(1);
		match(ID);
#line 867 "Vrml97Parser.g"
		
		n.reset(scope->find_node(id1->getText()));
		if (!n) {
		throw SemanticException("Node \"" + id1->getText()
		+ "\" has not been defined in this "
		+ "scope.",
		this->uri,
		id1->getLine(),
		id1->getColumn());
		}
		
#line 1474 "Vrml97Parser.cpp"
		break;
	}
	case ID:
	{
		n=protoNode(browser,
                    scope,
                    proto_interfaces,
                    is_map,
                    routes,
                    std::string());
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return n;
}

void Vrml97Parser::protoBodyStatement(
	openvrml::browser & browser,
                   const scope_ptr & scope,
                   const node_interface_set & interfaces,
                   std::vector<node_ptr> & impl_nodes,
                   proto_node_class::is_map_t & is_map,
                   proto_node_class::routes_t & routes
) {
#line 830 "Vrml97Parser.g"
	
	assert(scope);
	assert(impl_nodes.size() == 1);
	
	node_ptr n;
	
#line 1510 "Vrml97Parser.cpp"
	
	switch ( LA(1)) {
	case ID:
	case KEYWORD_DEF:
	case KEYWORD_USE:
	{
		n=protoNodeStatement(browser, scope, interfaces, is_map, routes);
#line 843 "Vrml97Parser.g"
		
		assert(n);
		impl_nodes.push_back(n);
		
#line 1523 "Vrml97Parser.cpp"
		break;
	}
	case KEYWORD_EXTERNPROTO:
	case KEYWORD_PROTO:
	{
		protoStatement(browser, scope);
		break;
	}
	case KEYWORD_ROUTE:
	{
		protoRouteStatement(*scope, routes);
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
}

void Vrml97Parser::protoRouteStatement(
	const openvrml::scope & scope,
                    proto_node_class::routes_t & routes
) {
	ANTLR_USE_NAMESPACE(antlr)RefToken  from_node_id = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  eventout_id = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  to_node_id = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  eventin_id = ANTLR_USE_NAMESPACE(antlr)nullToken;
	
	match(KEYWORD_ROUTE);
	from_node_id = LT(1);
	match(ID);
	match(PERIOD);
	eventout_id = LT(1);
	match(ID);
	match(KEYWORD_TO);
	to_node_id = LT(1);
	match(ID);
	match(PERIOD);
	eventin_id = LT(1);
	match(ID);
#line 1014 "Vrml97Parser.g"
	
	using std::runtime_error;
	using antlr::SemanticException;
	
	openvrml::node * const from_node =
	scope.find_node(from_node_id->getText());
	if (!from_node) {
	throw SemanticException("Node \"" + from_node_id->getText()
	+ "\" has not been defined in this "
	"scope.",
	this->uri,
	from_node_id->getLine(),
	from_node_id->getColumn());
	}
	
	openvrml::node * const to_node =
	scope.find_node(to_node_id->getText());
	if (!to_node) {
	throw SemanticException("Node \"" + to_node_id->getText()
	+ "\" has not been defined in this "
	"scope.",
	this->uri,
	to_node_id->getLine(),
	to_node_id->getColumn());
	}
	
	//
	// Routes outside PROTO definitions get added with add_route, which
	// does the appropriate validation.  However, in a PROTO
	// definition, we don't actually establish a route; the actual
	// routes (event_listeners) are established when the PROTO is
	// instantiated.  So, we need to do validation here.
	//
	const node_interface_set & from_node_interfaces =
	from_node->type.interfaces();
	const node_interface_set::const_iterator from_interface =
	find_if(from_node_interfaces.begin(),
	from_node_interfaces.end(),
	bind2nd(node_interface_matches_eventout(),
	eventout_id->getText()));
	if (from_interface == from_node_interfaces.end()) {
	throw SemanticException(from_node->type.id + " node has no "
	"eventOut \"" + eventout_id->getText()
	+ "\".",
	this->uri,
	eventout_id->getLine(),
	eventout_id->getColumn());
	}
	
	const node_interface_set & to_node_interfaces =
	to_node->type.interfaces();
	const node_interface_set::const_iterator to_interface =
	find_if(to_node_interfaces.begin(), to_node_interfaces.end(),
	bind2nd(node_interface_matches_eventin(),
	eventin_id->getText()));
	if (to_interface == to_node_interfaces.end()) {
	throw SemanticException(to_node->type.id + " node has no "
	"eventIn \"" + eventin_id->getText()
	+ "\".",
	this->uri,
	eventin_id->getLine(),
	eventin_id->getColumn());
	}
	
	if (to_interface->field_type != from_interface->field_type) {
	throw SemanticException("Mismatch between eventOut and "
	"eventIn types.",
	this->uri,
	eventin_id->getLine(),
	eventin_id->getColumn());
	}
	
	const proto_node_class::route r(*from_node, eventout_id->getText(),
	*to_node, eventin_id->getText());
	routes.push_back(r);
	
#line 1642 "Vrml97Parser.cpp"
}

node_ptr  Vrml97Parser::protoNode(
	openvrml::browser & browser,
          const scope_ptr & scope,
          const node_interface_set & proto_interfaces,
          proto_node_class::is_map_t & is_map,
          proto_node_class::routes_t & routes,
          const std::string & nodeId
) {
#line 1255 "Vrml97Parser.g"
	node_ptr n;
#line 1655 "Vrml97Parser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  scriptId = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  nodeTypeId = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 1255 "Vrml97Parser.g"
	
	using antlr::SemanticException;
	
	initial_value_map initial_values;
	node_interface_set interfaces;
	is_list is_mappings;
	node_type_ptr nodeType;
	
#line 1667 "Vrml97Parser.cpp"
	
	{
	if (((LA(1) == ID))&&( !LT(1)->getText().compare("Script") )) {
		scriptId = LT(1);
		match(ID);
		match(LBRACE);
		{ // ( ... )*
		for (;;) {
			switch ( LA(1)) {
			case ID:
			case KEYWORD_EXTERNPROTO:
			case KEYWORD_PROTO:
			case KEYWORD_ROUTE:
			{
				protoNodeBodyElement(browser,
                                     scope,
                                     proto_interfaces,
                                     is_map,
                                     routes,
                                     script_node_interface_set,
                                     initial_values,
                                     is_mappings);
				break;
			}
			case KEYWORD_EVENTIN:
			case KEYWORD_EVENTOUT:
			case KEYWORD_FIELD:
			{
				protoScriptInterfaceDeclaration(browser,
                                                  scope,
                                                  proto_interfaces,
                                                  is_map,
                                                  routes,
                                                  interfaces,
                                                  initial_values,
                                                  is_mappings);
				break;
			}
			default:
			{
				goto _loop41;
			}
			}
		}
		_loop41:;
		} // ( ... )*
		match(RBRACE);
#line 1289 "Vrml97Parser.g"
		
		n.reset(new script_node(browser.script_node_class_,
		scope,
		interfaces,
		initial_values));
		if (!nodeId.empty()) { n->id(nodeId); }
		
#line 1723 "Vrml97Parser.cpp"
	}
	else if ((LA(1) == ID)) {
		nodeTypeId = LT(1);
		match(ID);
#line 1297 "Vrml97Parser.g"
		
		nodeType = scope->find_type(nodeTypeId->getText());
		if (!nodeType) {
		throw SemanticException("Unknown node type \""
		+ nodeTypeId->getText() + "\".",
		this->uri,
		nodeTypeId->getLine(),
		nodeTypeId->getColumn());
		}
		
#line 1739 "Vrml97Parser.cpp"
		match(LBRACE);
		{ // ( ... )*
		for (;;) {
			if ((_tokenSet_1.member(LA(1)))) {
				protoNodeBodyElement(browser,
                                         scope,
                                         proto_interfaces,
                                         is_map,
                                         routes,
                                         nodeType->interfaces(),
                                         initial_values,
                                         is_mappings);
			}
			else {
				goto _loop43;
			}
			
		}
		_loop43:;
		} // ( ... )*
		match(RBRACE);
#line 1314 "Vrml97Parser.g"
		
		n = nodeType->create_node(scope, initial_values);
		if (!nodeId.empty()) { n->id(nodeId); }
		
#line 1766 "Vrml97Parser.cpp"
	}
	else {
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	
	}
#line 1318 "Vrml97Parser.g"
	
	for (is_list::const_iterator is_mapping = is_mappings.begin();
	is_mapping != is_mappings.end();
	++is_mapping) {
	typedef proto_node_class::is_target is_target;
	is_map.insert(make_pair(is_mapping->second,
	is_target(*n, is_mapping->first)));
	}
	
#line 1783 "Vrml97Parser.cpp"
	return n;
}

void Vrml97Parser::externInterfaceDeclaration(
	openvrml::node_interface_set & interfaces
) {
	ANTLR_USE_NAMESPACE(antlr)RefToken  id = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 927 "Vrml97Parser.g"
	
	using openvrml::node_interface;
	using openvrml::field_value;
	using antlr::SemanticException;
	node_interface::type_id it(node_interface::invalid_type_id);
	field_value::type_id ft(field_value::invalid_type_id);
	
#line 1799 "Vrml97Parser.cpp"
	
	it=interfaceType();
	ft=fieldType();
	id = LT(1);
	match(ID);
#line 936 "Vrml97Parser.g"
	
	const node_interface interface(it, ft, id->getText());
	try {
	add_interface(interfaces, interface);
	} catch (std::invalid_argument & ex) {
		throw SemanticException(ex.what(),
	this->uri,
	id->getLine(),
	id->getColumn());
	}
	
#line 1817 "Vrml97Parser.cpp"
}

mfstring  Vrml97Parser::externprotoUrlList() {
#line 955 "Vrml97Parser.g"
	mfstring urlList;
#line 1823 "Vrml97Parser.cpp"
#line 955 "Vrml97Parser.g"
	
	using std::string;
	using openvrml::mfstring;
	
	string s;
	
#line 1831 "Vrml97Parser.cpp"
	
	switch ( LA(1)) {
	case STRING:
	{
		s=stringValue();
#line 963 "Vrml97Parser.g"
		urlList.value.push_back(s);
#line 1839 "Vrml97Parser.cpp"
		break;
	}
	case LBRACKET:
	{
		match(LBRACKET);
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == STRING)) {
				s=stringValue();
#line 964 "Vrml97Parser.g"
				urlList.value.push_back(s);
#line 1851 "Vrml97Parser.cpp"
			}
			else {
				goto _loop27;
			}
			
		}
		_loop27:;
		} // ( ... )*
		match(RBRACKET);
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return urlList;
}

node_interface::type_id  Vrml97Parser::interfaceType() {
#line 949 "Vrml97Parser.g"
	node_interface::type_id interface_type;
#line 1874 "Vrml97Parser.cpp"
	
	switch ( LA(1)) {
	case KEYWORD_EVENTIN:
	case KEYWORD_EVENTOUT:
	{
		interface_type=eventInterfaceType();
		break;
	}
	case KEYWORD_EXPOSEDFIELD:
	case KEYWORD_FIELD:
	{
		interface_type=fieldInterfaceType();
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return interface_type;
}

std::string  Vrml97Parser::stringValue() {
#line 1894 "Vrml97Parser.g"
	std::string str;
#line 1900 "Vrml97Parser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  s = ANTLR_USE_NAMESPACE(antlr)nullToken;
	
	s = LT(1);
	match(STRING);
#line 1897 "Vrml97Parser.g"
	
	const std::string temp(s->getText());
	str = std::string(temp.begin() + 1, temp.end() - 1);
	
#line 1910 "Vrml97Parser.cpp"
	return str;
}

void Vrml97Parser::nodeBodyElement(
	browser & b,
                const scope_ptr & scope,
                const node_interface_set & interfaces,
                initial_value_map & initial_values
) {
	ANTLR_USE_NAMESPACE(antlr)RefToken  id = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 1152 "Vrml97Parser.g"
	
	using std::find_if;
	using std::bind2nd;
	using antlr::SemanticException;
	field_value::type_id ft = field_value::invalid_type_id;
	field_value_ptr fv;
	
#line 1929 "Vrml97Parser.cpp"
	
	switch ( LA(1)) {
	case ID:
	{
		id = LT(1);
		match(ID);
#line 1164 "Vrml97Parser.g"
		
		node_interface_set::const_iterator interface =
		find_if(interfaces.begin(), interfaces.end(),
		bind2nd(node_interface_matches_field(),
		id->getText()));
		if (interface == interfaces.end()) {
		interface =
		find_if(interfaces.begin(), interfaces.end(),
		bind2nd(node_interface_matches_exposedfield(),
		id->getText()));
		}
		if (interface == interfaces.end()) {
		throw SemanticException("Node has no field or exposedField \""
		+ id->getText() + "\"",
		this->uri,
		id->getLine(),
		id->getColumn());
		}
		ft = interface->field_type;
		
#line 1957 "Vrml97Parser.cpp"
		fv=fieldValue(b, scope, ft);
#line 1183 "Vrml97Parser.g"
		
		assert(fv);
		const bool succeeded =
		initial_values.insert(make_pair(id->getText(), fv)).second;
		if (!succeeded) {
		throw SemanticException("Value for " + id->getText()
		+ " already declared.");
		}
		
#line 1969 "Vrml97Parser.cpp"
		break;
	}
	case KEYWORD_ROUTE:
	{
		routeStatement(*scope);
		break;
	}
	case KEYWORD_EXTERNPROTO:
	case KEYWORD_PROTO:
	{
		protoStatement(b, scope);
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
}

void Vrml97Parser::scriptInterfaceDeclaration(
	browser & b,
                           const scope_ptr & scope,
                           node_interface_set & interfaces,
                           initial_value_map & initial_values
) {
	ANTLR_USE_NAMESPACE(antlr)RefToken  id = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 1196 "Vrml97Parser.g"
	
	using antlr::SemanticException;
	node_interface::type_id it(node_interface::invalid_type_id);
	field_value::type_id ft(field_value::invalid_type_id);
	
#line 2003 "Vrml97Parser.cpp"
	
	switch ( LA(1)) {
	case KEYWORD_EVENTIN:
	case KEYWORD_EVENTOUT:
	{
		it=eventInterfaceType();
		ft=fieldType();
		id = LT(1);
		match(ID);
#line 1206 "Vrml97Parser.g"
		
		const node_interface_set::const_iterator pos =
		find_interface(interfaces, id->getText());
		if (pos != interfaces.end()) {
		throw SemanticException("Interface \"" + id->getText()
		+ "\" already declared for Script "
		"node.",
		this->uri,
		id->getLine(),
		id->getColumn());
		}
		add_interface(interfaces, node_interface(it, ft, id->getText()));
		
#line 2027 "Vrml97Parser.cpp"
		break;
	}
	case KEYWORD_FIELD:
	{
		scriptFieldInterfaceDeclaration(b, scope, interfaces, initial_values);
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
}

void Vrml97Parser::scriptFieldInterfaceDeclaration(
	browser & b,
                                const scope_ptr & scope,
                                node_interface_set & interfaces,
                                initial_value_map & initial_values
) {
	ANTLR_USE_NAMESPACE(antlr)RefToken  id = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 1222 "Vrml97Parser.g"
	
	using std::find_if;
	using antlr::SemanticException;
	
	field_value::type_id ft = field_value::invalid_type_id;
	field_value_ptr fv;
	
#line 2057 "Vrml97Parser.cpp"
	
	match(KEYWORD_FIELD);
	ft=fieldType();
	id = LT(1);
	match(ID);
	fv=fieldValue(b, scope, ft);
#line 1234 "Vrml97Parser.g"
	
	assert(fv);
	bool succeeded =
	interfaces.insert(node_interface(node_interface::field_id,
	ft,
	id->getText()))
	.second;
	if (!succeeded) {
	throw SemanticException("Interface \"" + id->getText()
	+ "\" already declared for Script "
	"node.",
	this->uri,
	id->getLine(),
	id->getColumn());
	}
	succeeded = initial_values.insert(make_pair(id->getText(), fv))
	.second;
	assert(succeeded);
	
#line 2084 "Vrml97Parser.cpp"
}

void Vrml97Parser::protoNodeBodyElement(
	openvrml::browser & browser,
                     const scope_ptr & scope,
                     const node_interface_set & proto_interfaces,
                     proto_node_class::is_map_t & is_map,
                     proto_node_class::routes_t & routes,
                     const node_interface_set & node_interfaces,
                     initial_value_map & initial_values,
                     is_list & is_mappings
) {
	ANTLR_USE_NAMESPACE(antlr)RefToken  interface_id = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 1329 "Vrml97Parser.g"
	
	using std::string;
	using antlr::SemanticException;
	
	field_value_ptr fv;
	
#line 2105 "Vrml97Parser.cpp"
	
	switch ( LA(1)) {
	case ID:
	{
		interface_id = LT(1);
		match(ID);
#line 1344 "Vrml97Parser.g"
		
		const node_interface_set::const_iterator impl_node_interface =
		find_interface(node_interfaces, interface_id->getText());
		if (impl_node_interface == node_interfaces.end()) {
		throw SemanticException("Node has no interface \""
		+ interface_id->getText() + "\".",
		this->uri,
		interface_id->getLine(),
		interface_id->getColumn());
		}
		
#line 2124 "Vrml97Parser.cpp"
		{
		if (((_tokenSet_5.member(LA(1))))&&(impl_node_interface->type == node_interface::field_id
            || impl_node_interface->type == node_interface::exposedfield_id)) {
			{
			switch ( LA(1)) {
			case LBRACKET:
			case ID:
			case INTEGER:
			case HEX_INTEGER:
			case REAL:
			case STRING:
			case KEYWORD_DEF:
			case KEYWORD_FALSE:
			case KEYWORD_NULL:
			case KEYWORD_TRUE:
			case KEYWORD_USE:
			{
				fv=protoFieldValue(browser,
                                   scope,
                                   proto_interfaces,
                                   is_map,
                                   routes,
                                   impl_node_interface->field_type);
#line 1362 "Vrml97Parser.g"
				
				assert(fv);
				bool succeeded =
				initial_values.insert(
				make_pair(interface_id->getText(), fv)).second;
				assert(succeeded);
				
#line 2156 "Vrml97Parser.cpp"
				break;
			}
			case KEYWORD_IS:
			{
				isStatement(impl_node_interface->id, is_mappings);
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
			}
			}
			}
		}
		else if ((LA(1) == KEYWORD_IS)) {
			isStatement(impl_node_interface->id, is_mappings);
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		
		}
		break;
	}
	case KEYWORD_ROUTE:
	{
		protoRouteStatement(*scope, routes);
		break;
	}
	case KEYWORD_EXTERNPROTO:
	case KEYWORD_PROTO:
	{
		protoStatement(browser, scope);
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
}

void Vrml97Parser::protoScriptInterfaceDeclaration(
	openvrml::browser & browser,
                                const openvrml::scope_ptr & scope,
                                const node_interface_set & proto_interfaces,
                                proto_node_class::is_map_t & is_map,
                                proto_node_class::routes_t & routes,
                                node_interface_set & interfaces,
                                initial_value_map & initial_values,
                                is_list & is_mappings
) {
	ANTLR_USE_NAMESPACE(antlr)RefToken  id = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 1391 "Vrml97Parser.g"
	
	using antlr::SemanticException;
	node_interface::type_id it;
	field_value::type_id ft;
	
#line 2216 "Vrml97Parser.cpp"
	
	switch ( LA(1)) {
	case KEYWORD_EVENTIN:
	case KEYWORD_EVENTOUT:
	{
		it=eventInterfaceType();
		ft=fieldType();
		id = LT(1);
		match(ID);
#line 1405 "Vrml97Parser.g"
		
		const node_interface_set::const_iterator pos =
		find_interface(interfaces, id->getText());
		if (pos != interfaces.end()) {
		throw SemanticException("Interface \"" + id->getText()
		+ "\" already declared for Script "
		"node.",
		this->uri,
		id->getLine(),
		id->getColumn());
		}
		const node_interface_set::const_iterator interface =
		add_interface(interfaces,
		node_interface(it, ft, id->getText()));
		
#line 2242 "Vrml97Parser.cpp"
		{
		switch ( LA(1)) {
		case KEYWORD_IS:
		{
			isStatement(interface->id, is_mappings);
			break;
		}
		case RBRACE:
		case ID:
		case KEYWORD_EVENTIN:
		case KEYWORD_EVENTOUT:
		case KEYWORD_EXTERNPROTO:
		case KEYWORD_FIELD:
		case KEYWORD_PROTO:
		case KEYWORD_ROUTE:
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
		}
		}
		}
		break;
	}
	case KEYWORD_FIELD:
	{
		protoScriptFieldInterfaceDeclaration(browser,
                                             scope,
                                             proto_interfaces,
                                             is_map,
                                             routes,
                                             interfaces,
                                             initial_values,
                                             is_mappings);
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
}

field_value_ptr  Vrml97Parser::protoFieldValue(
	openvrml::browser & browser,
                const scope_ptr & scope,
                const node_interface_set & proto_interfaces,
                proto_node_class::is_map_t & is_map,
                proto_node_class::routes_t & routes,
                const field_value::type_id ft
) {
#line 1537 "Vrml97Parser.g"
	field_value_ptr fv;
#line 2298 "Vrml97Parser.cpp"
	
	if (((_tokenSet_3.member(LA(1))))&&( (ft == field_value::sfnode_id) || (ft == field_value::mfnode_id) )) {
		fv=protoNodeFieldValue(browser,
                               scope,
                               proto_interfaces,
                               is_map,
                               routes,
                               ft);
#line 1551 "Vrml97Parser.g"
		
		assert(fv);
		
#line 2311 "Vrml97Parser.cpp"
	}
	else if ((_tokenSet_4.member(LA(1)))) {
		fv=nonNodeFieldValue(ft);
#line 1554 "Vrml97Parser.g"
		assert(fv);
#line 2317 "Vrml97Parser.cpp"
	}
	else {
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	
	return fv;
}

void Vrml97Parser::isStatement(
	const std::string & impl_node_interface_id,
            is_list & is_mappings
) {
	ANTLR_USE_NAMESPACE(antlr)RefToken  id = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 1377 "Vrml97Parser.g"
	
	using std::string;
	using boost::lexical_cast;
	using antlr::SemanticException;
	
#line 2337 "Vrml97Parser.cpp"
	
	match(KEYWORD_IS);
	id = LT(1);
	match(ID);
#line 1385 "Vrml97Parser.g"
	
	is_mappings.insert(make_pair(impl_node_interface_id,
	id->getText()));
	
#line 2347 "Vrml97Parser.cpp"
}

void Vrml97Parser::protoScriptFieldInterfaceDeclaration(
	
    openvrml::browser & browser,
    const scope_ptr & scope,
    const node_interface_set & proto_interfaces,
    proto_node_class::is_map_t & is_map,
    proto_node_class::routes_t & routes,
    node_interface_set & interfaces,
    initial_value_map & initial_values,
    is_list & is_mappings
) {
	ANTLR_USE_NAMESPACE(antlr)RefToken  id = ANTLR_USE_NAMESPACE(antlr)nullToken;
#line 1430 "Vrml97Parser.g"
	
	using std::auto_ptr;
	using std::find_if;
	using std::string;
	using boost::lexical_cast;
	using boost::shared_ptr;
	using antlr::SemanticException;
	
	field_value::type_id ft;
	field_value_ptr fv;
	bool succeeded;
	
#line 2375 "Vrml97Parser.cpp"
	
	match(KEYWORD_FIELD);
	ft=fieldType();
	id = LT(1);
	match(ID);
#line 1452 "Vrml97Parser.g"
	
	succeeded =
	interfaces.insert(node_interface(node_interface::field_id,
	ft,
	id->getText()))
	.second;
	if (!succeeded) {
	throw SemanticException("Interface \"" + id->getText()
	+ "\" already declared for Script "
	"node.",
	this->uri,
	id->getLine(),
	id->getColumn());
	}
	
#line 2397 "Vrml97Parser.cpp"
	{
	switch ( LA(1)) {
	case LBRACKET:
	case ID:
	case INTEGER:
	case HEX_INTEGER:
	case REAL:
	case STRING:
	case KEYWORD_DEF:
	case KEYWORD_FALSE:
	case KEYWORD_NULL:
	case KEYWORD_TRUE:
	case KEYWORD_USE:
	{
		fv=protoFieldValue(browser,
                               scope,
                               proto_interfaces,
                               is_map,
                               routes,
                               ft);
#line 1472 "Vrml97Parser.g"
		
		assert(fv);
		succeeded = initial_values.insert(make_pair(id->getText(),
		fv))
		.second;
		assert(succeeded);
		
#line 2426 "Vrml97Parser.cpp"
		break;
	}
	case KEYWORD_IS:
	{
		isStatement(id->getText(), is_mappings);
#line 1479 "Vrml97Parser.g"
		
		//
		// The field needs some default value as a placeholder. This
		// is never actually used in a PROTO instance.
		//
		auto_ptr<field_value> value = field_value::create(ft);
		succeeded =
		initial_values.insert(
		make_pair(id->getText(),
		shared_ptr<field_value>(value)))
		.second;
		assert(succeeded);
		
#line 2446 "Vrml97Parser.cpp"
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	}
}

openvrml::field_value_ptr  Vrml97Parser::nodeFieldValue(
	openvrml::browser & browser,
               const openvrml::scope_ptr & scope,
               openvrml::field_value::type_id ft
) {
#line 1583 "Vrml97Parser.g"
	openvrml::field_value_ptr fv;
#line 2464 "Vrml97Parser.cpp"
#line 1583 "Vrml97Parser.g"
	
	using openvrml::field_value;
	
#line 2469 "Vrml97Parser.cpp"
	
	if (((_tokenSet_6.member(LA(1))))&&( ft == field_value::sfnode_id )) {
		fv=sfNodeValue(browser, scope);
	}
	else if ((_tokenSet_7.member(LA(1)))) {
		fv=mfNodeValue(browser, scope);
	}
	else {
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	
	return fv;
}

openvrml::field_value_ptr  Vrml97Parser::nonNodeFieldValue(
	openvrml::field_value::type_id ft
) {
#line 1557 "Vrml97Parser.g"
	openvrml::field_value_ptr fv = openvrml::field_value_ptr();
#line 2489 "Vrml97Parser.cpp"
#line 1557 "Vrml97Parser.g"
	
	using openvrml::field_value;
	
#line 2494 "Vrml97Parser.cpp"
	
	if (((LA(1) == KEYWORD_FALSE || LA(1) == KEYWORD_TRUE))&&( ft == field_value::sfbool_id )) {
		fv=sfBoolValue();
	}
	else if (((LA(1) == INTEGER || LA(1) == REAL))&&( ft == field_value::sfcolor_id )) {
		fv=sfColorValue();
	}
	else if (((LA(1) == INTEGER || LA(1) == REAL))&&( ft == field_value::sffloat_id )) {
		fv=sfFloatValue();
	}
	else if (((LA(1) == INTEGER || LA(1) == HEX_INTEGER))&&( ft == field_value::sfimage_id )) {
		fv=sfImageValue();
	}
	else if (((LA(1) == INTEGER || LA(1) == HEX_INTEGER))&&( ft == field_value::sfint32_id )) {
		fv=sfInt32Value();
	}
	else if (((LA(1) == INTEGER || LA(1) == REAL))&&( ft == field_value::sfrotation_id )) {
		fv=sfRotationValue();
	}
	else if (((LA(1) == STRING))&&( ft == field_value::sfstring_id )) {
		fv=sfStringValue();
	}
	else if (((LA(1) == INTEGER || LA(1) == REAL))&&( ft == field_value::sftime_id )) {
		fv=sfTimeValue();
	}
	else if (((LA(1) == INTEGER || LA(1) == REAL))&&( ft == field_value::sfvec2f_id )) {
		fv=sfVec2fValue();
	}
	else if (((LA(1) == INTEGER || LA(1) == REAL))&&( ft == field_value::sfvec3f_id )) {
		fv=sfVec3fValue();
	}
	else if (((LA(1) == LBRACKET || LA(1) == INTEGER || LA(1) == REAL))&&( ft == field_value::mfcolor_id )) {
		fv=mfColorValue();
	}
	else if (((LA(1) == LBRACKET || LA(1) == INTEGER || LA(1) == REAL))&&( ft == field_value::mffloat_id )) {
		fv=mfFloatValue();
	}
	else if (((LA(1) == LBRACKET || LA(1) == INTEGER || LA(1) == HEX_INTEGER))&&( ft == field_value::mfint32_id )) {
		fv=mfInt32Value();
	}
	else if (((LA(1) == LBRACKET || LA(1) == INTEGER || LA(1) == REAL))&&( ft == field_value::mfrotation_id )) {
		fv=mfRotationValue();
	}
	else if (((LA(1) == LBRACKET || LA(1) == STRING))&&( ft == field_value::mfstring_id )) {
		fv=mfStringValue();
	}
	else if (((LA(1) == LBRACKET || LA(1) == INTEGER || LA(1) == REAL))&&( ft == field_value::mftime_id )) {
		fv=mfTimeValue();
	}
	else if (((LA(1) == LBRACKET || LA(1) == INTEGER || LA(1) == REAL))&&( ft == field_value::mfvec2f_id )) {
		fv=mfVec2fValue();
	}
	else if ((LA(1) == LBRACKET || LA(1) == INTEGER || LA(1) == REAL)) {
		fv=mfVec3fValue();
	}
	else {
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	
	return fv;
}

field_value_ptr  Vrml97Parser::protoNodeFieldValue(
	openvrml::browser & browser,
                    const scope_ptr & scope,
                    const node_interface_set & proto_interfaces,
                    proto_node_class::is_map_t & is_map,
                    proto_node_class::routes_t & routes,
                    field_value::type_id ft
) {
#line 1595 "Vrml97Parser.g"
	field_value_ptr fv;
#line 2567 "Vrml97Parser.cpp"
	
	if (((_tokenSet_6.member(LA(1))))&&( ft == field_value::sfnode_id )) {
		fv=protoSfNodeValue(browser, scope, proto_interfaces, is_map, routes);
	}
	else if ((_tokenSet_7.member(LA(1)))) {
		fv=protoMfNodeValue(browser, scope, proto_interfaces, is_map, routes);
	}
	else {
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	
	return fv;
}

openvrml::field_value_ptr  Vrml97Parser::sfBoolValue() {
#line 1608 "Vrml97Parser.g"
	openvrml::field_value_ptr sbv;
#line 2585 "Vrml97Parser.cpp"
#line 1608 "Vrml97Parser.g"
	
	bool val(false);
	
#line 2590 "Vrml97Parser.cpp"
	
	val=boolValue();
#line 1613 "Vrml97Parser.g"
	sbv.reset(new sfbool(val));
#line 2595 "Vrml97Parser.cpp"
	return sbv;
}

openvrml::field_value_ptr  Vrml97Parser::sfColorValue() {
#line 1622 "Vrml97Parser.g"
	openvrml::field_value_ptr scv;
#line 2602 "Vrml97Parser.cpp"
#line 1622 "Vrml97Parser.g"
	
	color c;
	
#line 2607 "Vrml97Parser.cpp"
	
	colorValue(c);
#line 1627 "Vrml97Parser.g"
	scv.reset(new sfcolor(c));
#line 2612 "Vrml97Parser.cpp"
	return scv;
}

openvrml::field_value_ptr  Vrml97Parser::sfFloatValue() {
#line 1670 "Vrml97Parser.g"
	openvrml::field_value_ptr sfv;
#line 2619 "Vrml97Parser.cpp"
#line 1670 "Vrml97Parser.g"
	
	float f;
	
#line 2624 "Vrml97Parser.cpp"
	
	f=floatValue();
#line 1675 "Vrml97Parser.g"
	sfv.reset(new sffloat(f));
#line 2629 "Vrml97Parser.cpp"
	return sfv;
}

openvrml::field_value_ptr  Vrml97Parser::sfImageValue() {
#line 1696 "Vrml97Parser.g"
	openvrml::field_value_ptr siv;
#line 2636 "Vrml97Parser.cpp"
#line 1696 "Vrml97Parser.g"
	
	using antlr::SemanticException;
	size_t x, y, comp;
	int32 pixel;
	size_t pixel_index = 0;
	
#line 2644 "Vrml97Parser.cpp"
	
	x=intValue();
	y=intValue();
	comp=intValue();
#line 1704 "Vrml97Parser.g"
	
	image img(x, y, comp);
	
#line 2653 "Vrml97Parser.cpp"
	{ // ( ... )*
	for (;;) {
		if ((LA(1) == INTEGER || LA(1) == HEX_INTEGER)) {
			pixel=intValue();
#line 1707 "Vrml97Parser.g"
			
			if (!(pixel_index < x * y)) {
			throw SemanticException("Too many pixel values for "
			"SFImage.",
			this->uri,
			LT(1)->getLine(),
			LT(1)->getColumn());
			}
			img.pixel(pixel_index++, pixel);
			
#line 2669 "Vrml97Parser.cpp"
		}
		else {
			goto _loop73;
		}
		
	}
	_loop73:;
	} // ( ... )*
#line 1717 "Vrml97Parser.g"
	
	if (pixel_index != x * y) {
	throw antlr::SemanticException("Insufficient pixel values for "
	"SFImage.",
	this->uri,
	LT(1)->getLine(),
	LT(1)->getColumn());
	}
	siv.reset(new sfimage(img));
	
#line 2689 "Vrml97Parser.cpp"
	return siv;
}

openvrml::field_value_ptr  Vrml97Parser::sfInt32Value() {
#line 1729 "Vrml97Parser.g"
	openvrml::field_value_ptr siv;
#line 2696 "Vrml97Parser.cpp"
#line 1729 "Vrml97Parser.g"
	
	long i;
	
#line 2701 "Vrml97Parser.cpp"
	
	i=intValue();
#line 1734 "Vrml97Parser.g"
	siv.reset(new sfint32(i));
#line 2706 "Vrml97Parser.cpp"
	return siv;
}

openvrml::field_value_ptr  Vrml97Parser::sfRotationValue() {
#line 1831 "Vrml97Parser.g"
	openvrml::field_value_ptr srv;
#line 2713 "Vrml97Parser.cpp"
#line 1831 "Vrml97Parser.g"
	rotation r;
#line 2716 "Vrml97Parser.cpp"
	
	rotationValue(r);
#line 1834 "Vrml97Parser.g"
	srv.reset(new sfrotation(r));
#line 2721 "Vrml97Parser.cpp"
	return srv;
}

openvrml::field_value_ptr  Vrml97Parser::sfStringValue() {
#line 1876 "Vrml97Parser.g"
	openvrml::field_value_ptr ssv;
#line 2728 "Vrml97Parser.cpp"
#line 1876 "Vrml97Parser.g"
	std::string s;
#line 2731 "Vrml97Parser.cpp"
	
	s=stringValue();
#line 1879 "Vrml97Parser.g"
	ssv.reset(new sfstring(s));
#line 2736 "Vrml97Parser.cpp"
	return ssv;
}

openvrml::field_value_ptr  Vrml97Parser::sfTimeValue() {
#line 1903 "Vrml97Parser.g"
	openvrml::field_value_ptr stv;
#line 2743 "Vrml97Parser.cpp"
#line 1903 "Vrml97Parser.g"
	double t(0.0);
#line 2746 "Vrml97Parser.cpp"
	
	t=doubleValue();
#line 1906 "Vrml97Parser.g"
	stv.reset(new sftime(t));
#line 2751 "Vrml97Parser.cpp"
	return stv;
}

openvrml::field_value_ptr  Vrml97Parser::sfVec2fValue() {
#line 1926 "Vrml97Parser.g"
	openvrml::field_value_ptr svv;
#line 2758 "Vrml97Parser.cpp"
#line 1926 "Vrml97Parser.g"
	vec2f v;
#line 2761 "Vrml97Parser.cpp"
	
	vec2fValue(v);
#line 1929 "Vrml97Parser.g"
	svv.reset(new sfvec2f(v));
#line 2766 "Vrml97Parser.cpp"
	return svv;
}

openvrml::field_value_ptr  Vrml97Parser::sfVec3fValue() {
#line 1953 "Vrml97Parser.g"
	openvrml::field_value_ptr svv;
#line 2773 "Vrml97Parser.cpp"
#line 1953 "Vrml97Parser.g"
	
	vec3f v;
	
#line 2778 "Vrml97Parser.cpp"
	
	vec3fValue(v);
#line 1958 "Vrml97Parser.g"
	svv.reset(new sfvec3f(v));
#line 2783 "Vrml97Parser.cpp"
	return svv;
}

openvrml::field_value_ptr  Vrml97Parser::mfColorValue() {
#line 1630 "Vrml97Parser.g"
	openvrml::field_value_ptr mcv =
            openvrml::field_value_ptr(new mfcolor);
#line 2791 "Vrml97Parser.cpp"
#line 1630 "Vrml97Parser.g"
	
	color c;
	mfcolor & colors = static_cast<mfcolor &>(*mcv);
	
#line 2797 "Vrml97Parser.cpp"
	
	switch ( LA(1)) {
	case INTEGER:
	case REAL:
	{
		colorValue(c);
#line 1638 "Vrml97Parser.g"
		colors.value.push_back(c);
#line 2806 "Vrml97Parser.cpp"
		break;
	}
	case LBRACKET:
	{
		match(LBRACKET);
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == INTEGER || LA(1) == REAL)) {
				colorValue(c);
#line 1639 "Vrml97Parser.g"
				colors.value.push_back(c);
#line 2818 "Vrml97Parser.cpp"
			}
			else {
				goto _loop63;
			}
			
		}
		_loop63:;
		} // ( ... )*
		match(RBRACKET);
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return mcv;
}

openvrml::field_value_ptr  Vrml97Parser::mfFloatValue() {
#line 1678 "Vrml97Parser.g"
	openvrml::field_value_ptr mfv =
            openvrml::field_value_ptr(new mffloat);
#line 2842 "Vrml97Parser.cpp"
#line 1678 "Vrml97Parser.g"
	
	float f;
	mffloat & floats = static_cast<mffloat &>(*mfv);
	
#line 2848 "Vrml97Parser.cpp"
	
	switch ( LA(1)) {
	case INTEGER:
	case REAL:
	{
		f=floatValue();
#line 1686 "Vrml97Parser.g"
		floats.value.push_back(f);
#line 2857 "Vrml97Parser.cpp"
		break;
	}
	case LBRACKET:
	{
		match(LBRACKET);
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == INTEGER || LA(1) == REAL)) {
				f=floatValue();
#line 1687 "Vrml97Parser.g"
				floats.value.push_back(f);
#line 2869 "Vrml97Parser.cpp"
			}
			else {
				goto _loop69;
			}
			
		}
		_loop69:;
		} // ( ... )*
		match(RBRACKET);
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return mfv;
}

openvrml::field_value_ptr  Vrml97Parser::mfInt32Value() {
#line 1737 "Vrml97Parser.g"
	openvrml::field_value_ptr miv =
            openvrml::field_value_ptr(new mfint32);
#line 2893 "Vrml97Parser.cpp"
#line 1737 "Vrml97Parser.g"
	
	long i;
	mfint32 & int32s = static_cast<mfint32 &>(*miv);
	
#line 2899 "Vrml97Parser.cpp"
	
	switch ( LA(1)) {
	case INTEGER:
	case HEX_INTEGER:
	{
		i=intValue();
#line 1745 "Vrml97Parser.g"
		int32s.value.push_back(i);
#line 2908 "Vrml97Parser.cpp"
		break;
	}
	case LBRACKET:
	{
		match(LBRACKET);
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == INTEGER || LA(1) == HEX_INTEGER)) {
				i=intValue();
#line 1746 "Vrml97Parser.g"
				int32s.value.push_back(i);
#line 2920 "Vrml97Parser.cpp"
			}
			else {
				goto _loop77;
			}
			
		}
		_loop77:;
		} // ( ... )*
		match(RBRACKET);
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return miv;
}

field_value_ptr  Vrml97Parser::mfRotationValue() {
#line 1837 "Vrml97Parser.g"
	field_value_ptr mrv = field_value_ptr(new mfrotation);
#line 2943 "Vrml97Parser.cpp"
#line 1837 "Vrml97Parser.g"
	
	rotation r;
	mfrotation & rotations = static_cast<mfrotation &>(*mrv);
	
#line 2949 "Vrml97Parser.cpp"
	
	switch ( LA(1)) {
	case INTEGER:
	case REAL:
	{
		rotationValue(r);
#line 1844 "Vrml97Parser.g"
		rotations.value.push_back(r);
#line 2958 "Vrml97Parser.cpp"
		break;
	}
	case LBRACKET:
	{
		match(LBRACKET);
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == INTEGER || LA(1) == REAL)) {
				rotationValue(r);
#line 1845 "Vrml97Parser.g"
				rotations.value.push_back(r);
#line 2970 "Vrml97Parser.cpp"
			}
			else {
				goto _loop90;
			}
			
		}
		_loop90:;
		} // ( ... )*
		match(RBRACKET);
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return mrv;
}

openvrml::field_value_ptr  Vrml97Parser::mfStringValue() {
#line 1882 "Vrml97Parser.g"
	openvrml::field_value_ptr msv =
         openvrml::field_value_ptr(new mfstring);
#line 2994 "Vrml97Parser.cpp"
#line 1882 "Vrml97Parser.g"
	
	std::string s;
	mfstring & strings = static_cast<mfstring &>(*msv);
	
#line 3000 "Vrml97Parser.cpp"
	
	switch ( LA(1)) {
	case STRING:
	{
		s=stringValue();
#line 1890 "Vrml97Parser.g"
		strings.value.push_back(s);
#line 3008 "Vrml97Parser.cpp"
		break;
	}
	case LBRACKET:
	{
		match(LBRACKET);
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == STRING)) {
				s=stringValue();
#line 1891 "Vrml97Parser.g"
				strings.value.push_back(s);
#line 3020 "Vrml97Parser.cpp"
			}
			else {
				goto _loop95;
			}
			
		}
		_loop95:;
		} // ( ... )*
		match(RBRACKET);
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return msv;
}

openvrml::field_value_ptr  Vrml97Parser::mfTimeValue() {
#line 1909 "Vrml97Parser.g"
	openvrml::field_value_ptr mtv = openvrml::field_value_ptr(new mftime);
#line 3043 "Vrml97Parser.cpp"
#line 1909 "Vrml97Parser.g"
	
	double t;
	mftime & times = static_cast<mftime &>(*mtv);
	
#line 3049 "Vrml97Parser.cpp"
	
	switch ( LA(1)) {
	case INTEGER:
	case REAL:
	{
		t=doubleValue();
#line 1916 "Vrml97Parser.g"
		times.value.push_back(t);
#line 3058 "Vrml97Parser.cpp"
		break;
	}
	case LBRACKET:
	{
		match(LBRACKET);
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == INTEGER || LA(1) == REAL)) {
				t=doubleValue();
#line 1917 "Vrml97Parser.g"
				times.value.push_back(t);
#line 3070 "Vrml97Parser.cpp"
			}
			else {
				goto _loop100;
			}
			
		}
		_loop100:;
		} // ( ... )*
		match(RBRACKET);
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return mtv;
}

openvrml::field_value_ptr  Vrml97Parser::mfVec2fValue() {
#line 1932 "Vrml97Parser.g"
	openvrml::field_value_ptr mvv =
         openvrml::field_value_ptr(new mfvec2f);
#line 3094 "Vrml97Parser.cpp"
#line 1932 "Vrml97Parser.g"
	
	vec2f v;
	mfvec2f & vec2fs = static_cast<mfvec2f &>(*mvv);
	
#line 3100 "Vrml97Parser.cpp"
	
	switch ( LA(1)) {
	case INTEGER:
	case REAL:
	{
		vec2fValue(v);
#line 1940 "Vrml97Parser.g"
		vec2fs.value.push_back(v);
#line 3109 "Vrml97Parser.cpp"
		break;
	}
	case LBRACKET:
	{
		match(LBRACKET);
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == INTEGER || LA(1) == REAL)) {
				vec2fValue(v);
#line 1941 "Vrml97Parser.g"
				vec2fs.value.push_back(v);
#line 3121 "Vrml97Parser.cpp"
			}
			else {
				goto _loop105;
			}
			
		}
		_loop105:;
		} // ( ... )*
		match(RBRACKET);
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return mvv;
}

openvrml::field_value_ptr  Vrml97Parser::mfVec3fValue() {
#line 1961 "Vrml97Parser.g"
	openvrml::field_value_ptr mvv =
         openvrml::field_value_ptr(new mfvec3f);
#line 3145 "Vrml97Parser.cpp"
#line 1961 "Vrml97Parser.g"
	
	vec3f v;
	mfvec3f & vec3fs = static_cast<mfvec3f &>(*mvv);
	
#line 3151 "Vrml97Parser.cpp"
	
	switch ( LA(1)) {
	case INTEGER:
	case REAL:
	{
		vec3fValue(v);
#line 1969 "Vrml97Parser.g"
		vec3fs.value.push_back(v);
#line 3160 "Vrml97Parser.cpp"
		break;
	}
	case LBRACKET:
	{
		match(LBRACKET);
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == INTEGER || LA(1) == REAL)) {
				vec3fValue(v);
#line 1970 "Vrml97Parser.g"
				vec3fs.value.push_back(v);
#line 3172 "Vrml97Parser.cpp"
			}
			else {
				goto _loop110;
			}
			
		}
		_loop110:;
		} // ( ... )*
		match(RBRACKET);
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return mvv;
}

openvrml::field_value_ptr  Vrml97Parser::sfNodeValue(
	openvrml::browser & browser,
            const openvrml::scope_ptr & scope
) {
#line 1759 "Vrml97Parser.g"
	openvrml::field_value_ptr snv;
#line 3198 "Vrml97Parser.cpp"
#line 1759 "Vrml97Parser.g"
	
	openvrml::node_ptr n;
	
#line 3203 "Vrml97Parser.cpp"
	
	switch ( LA(1)) {
	case ID:
	case KEYWORD_DEF:
	case KEYWORD_USE:
	{
		n=nodeStatement(browser, scope);
#line 1766 "Vrml97Parser.g"
		snv.reset(new sfnode(n));
#line 3213 "Vrml97Parser.cpp"
		break;
	}
	case KEYWORD_NULL:
	{
		match(KEYWORD_NULL);
#line 1767 "Vrml97Parser.g"
		snv.reset(new sfnode);
#line 3221 "Vrml97Parser.cpp"
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return snv;
}

openvrml::field_value_ptr  Vrml97Parser::mfNodeValue(
	openvrml::browser & browser,
            const openvrml::scope_ptr & scope
) {
#line 1789 "Vrml97Parser.g"
	openvrml::field_value_ptr mnv = openvrml::field_value_ptr(new mfnode);
#line 3238 "Vrml97Parser.cpp"
#line 1789 "Vrml97Parser.g"
	
	openvrml::node_ptr n;
	mfnode & nodes = static_cast<mfnode &>(*mnv);
	
#line 3244 "Vrml97Parser.cpp"
	
	switch ( LA(1)) {
	case ID:
	case KEYWORD_DEF:
	case KEYWORD_USE:
	{
		n=nodeStatement(browser, scope);
#line 1797 "Vrml97Parser.g"
		if (n) { nodes.value.push_back(n); }
#line 3254 "Vrml97Parser.cpp"
		break;
	}
	case LBRACKET:
	{
		match(LBRACKET);
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == ID || LA(1) == KEYWORD_DEF || LA(1) == KEYWORD_USE)) {
				n=nodeStatement(browser, scope);
#line 1799 "Vrml97Parser.g"
				
				if (n) { nodes.value.push_back(n); }
				
#line 3268 "Vrml97Parser.cpp"
			}
			else {
				goto _loop83;
			}
			
		}
		_loop83:;
		} // ( ... )*
		match(RBRACKET);
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return mnv;
}

field_value_ptr  Vrml97Parser::protoSfNodeValue(
	openvrml::browser & browser,
                 const scope_ptr & scope,
                 const node_interface_set & proto_interfaces,
                 proto_node_class::is_map_t & is_map,
                 proto_node_class::routes_t & routes
) {
#line 1770 "Vrml97Parser.g"
	field_value_ptr snv;
#line 3297 "Vrml97Parser.cpp"
#line 1770 "Vrml97Parser.g"
	
	node_ptr n;
	
#line 3302 "Vrml97Parser.cpp"
	
	switch ( LA(1)) {
	case ID:
	case KEYWORD_DEF:
	case KEYWORD_USE:
	{
		n=protoNodeStatement(browser, scope, proto_interfaces, is_map, routes);
#line 1781 "Vrml97Parser.g"
		
		snv.reset(new sfnode(n));
		
#line 3314 "Vrml97Parser.cpp"
		break;
	}
	case KEYWORD_NULL:
	{
		match(KEYWORD_NULL);
#line 1784 "Vrml97Parser.g"
		
		snv.reset(new sfnode);
		
#line 3324 "Vrml97Parser.cpp"
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return snv;
}

field_value_ptr  Vrml97Parser::protoMfNodeValue(
	openvrml::browser & browser,
                 const scope_ptr & scope,
                 const node_interface_set & proto_interfaces,
                 proto_node_class::is_map_t & is_map,
                 proto_node_class::routes_t & routes
) {
#line 1805 "Vrml97Parser.g"
	field_value_ptr mnv = field_value_ptr(new mfnode);
#line 3344 "Vrml97Parser.cpp"
#line 1805 "Vrml97Parser.g"
	
	node_ptr n;
	mfnode & nodes = static_cast<mfnode &>(*mnv);
	
#line 3350 "Vrml97Parser.cpp"
	
	switch ( LA(1)) {
	case ID:
	case KEYWORD_DEF:
	case KEYWORD_USE:
	{
		n=protoNodeStatement(browser, scope, proto_interfaces, is_map, routes);
#line 1817 "Vrml97Parser.g"
		
		if (n) { nodes.value.push_back(n); }
		
#line 3362 "Vrml97Parser.cpp"
		break;
	}
	case LBRACKET:
	{
		match(LBRACKET);
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == ID || LA(1) == KEYWORD_DEF || LA(1) == KEYWORD_USE)) {
				n=protoNodeStatement(browser,
                                 scope,
                                 proto_interfaces,
                                 is_map,
                                 routes);
#line 1825 "Vrml97Parser.g"
				
				if (n) { nodes.value.push_back(n); }
				
#line 3380 "Vrml97Parser.cpp"
			}
			else {
				goto _loop86;
			}
			
		}
		_loop86:;
		} // ( ... )*
		match(RBRACKET);
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return mnv;
}

bool  Vrml97Parser::boolValue() {
#line 1616 "Vrml97Parser.g"
	bool val = false;
#line 3403 "Vrml97Parser.cpp"
	
	switch ( LA(1)) {
	case KEYWORD_TRUE:
	{
		match(KEYWORD_TRUE);
#line 1618 "Vrml97Parser.g"
		val = true;
#line 3411 "Vrml97Parser.cpp"
		break;
	}
	case KEYWORD_FALSE:
	{
		match(KEYWORD_FALSE);
#line 1619 "Vrml97Parser.g"
		val = false;
#line 3419 "Vrml97Parser.cpp"
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return val;
}

void Vrml97Parser::colorValue(
	color & c
) {
#line 1642 "Vrml97Parser.g"
	
	float r, g, b;
	
#line 3437 "Vrml97Parser.cpp"
	
	r=colorComponent();
	g=colorComponent();
	b=colorComponent();
#line 1647 "Vrml97Parser.g"
	c.r(r);
	c.g(g);
	c.b(b);
#line 3446 "Vrml97Parser.cpp"
}

float  Vrml97Parser::colorComponent() {
#line 1655 "Vrml97Parser.g"
	float val = 0.0f;
#line 3452 "Vrml97Parser.cpp"
	
	val=floatValue();
#line 1657 "Vrml97Parser.g"
	
	if (val < 0.0 || val > 1.0) {
	this->reportWarning("Color component values must be from 0 to "
	"1.");
	if (val < 0.0) {
	val = 0.0;
	} else if (val > 1.0) {
	val = 1.0;
	}
	}
	
#line 3467 "Vrml97Parser.cpp"
	return val;
}

float  Vrml97Parser::floatValue() {
#line 1690 "Vrml97Parser.g"
	float val;
#line 3474 "Vrml97Parser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  f0 = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  f1 = ANTLR_USE_NAMESPACE(antlr)nullToken;
	
	switch ( LA(1)) {
	case REAL:
	{
		f0 = LT(1);
		match(REAL);
#line 1692 "Vrml97Parser.g"
		std::istringstream(f0->getText()) >> val;
#line 3485 "Vrml97Parser.cpp"
		break;
	}
	case INTEGER:
	{
		f1 = LT(1);
		match(INTEGER);
#line 1693 "Vrml97Parser.g"
		std::istringstream(f1->getText()) >> val;
#line 3494 "Vrml97Parser.cpp"
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return val;
}

int32  Vrml97Parser::intValue() {
#line 1749 "Vrml97Parser.g"
	int32 val;
#line 3508 "Vrml97Parser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  i0 = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  i1 = ANTLR_USE_NAMESPACE(antlr)nullToken;
	
	switch ( LA(1)) {
	case INTEGER:
	{
		i0 = LT(1);
		match(INTEGER);
#line 1751 "Vrml97Parser.g"
		
		std::istringstream(i0->getText()) >> val;
		
#line 3521 "Vrml97Parser.cpp"
		break;
	}
	case HEX_INTEGER:
	{
		i1 = LT(1);
		match(HEX_INTEGER);
#line 1754 "Vrml97Parser.g"
		
		std::istringstream(i1->getText()) >> std::hex >> val;
		
#line 3532 "Vrml97Parser.cpp"
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return val;
}

void Vrml97Parser::rotationValue(
	rotation & r
) {
#line 1851 "Vrml97Parser.g"
	
	using openvrml_::fequal;
	float x, y, z, angle;
	
#line 3551 "Vrml97Parser.cpp"
	
	x=floatValue();
	y=floatValue();
	z=floatValue();
	angle=floatValue();
#line 1857 "Vrml97Parser.g"
	
	r.x(x);
	r.y(y);
	r.z(z);
	r.angle(angle);
	
	const float axisLength = r.axis().length();
	if (!fequal<float>()(axisLength, 1.0f)) {
	this->reportWarning("The axis component of a rotation must be "
	"a normalized vector.");
	if (fequal<float>()(axisLength, 0.0f)) {
	r.z(1.0);
	} else {
	r.axis(r.axis().normalize());
	}
	}
	
#line 3575 "Vrml97Parser.cpp"
}

double  Vrml97Parser::doubleValue() {
#line 1920 "Vrml97Parser.g"
	double val = 0.0;
#line 3581 "Vrml97Parser.cpp"
	ANTLR_USE_NAMESPACE(antlr)RefToken  d0 = ANTLR_USE_NAMESPACE(antlr)nullToken;
	ANTLR_USE_NAMESPACE(antlr)RefToken  d1 = ANTLR_USE_NAMESPACE(antlr)nullToken;
	
	switch ( LA(1)) {
	case REAL:
	{
		d0 = LT(1);
		match(REAL);
#line 1922 "Vrml97Parser.g"
		std::istringstream(d0->getText()) >> val;
#line 3592 "Vrml97Parser.cpp"
		break;
	}
	case INTEGER:
	{
		d1 = LT(1);
		match(INTEGER);
#line 1923 "Vrml97Parser.g"
		std::istringstream(d1->getText()) >> val;
#line 3601 "Vrml97Parser.cpp"
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltException(LT(1), getFilename());
	}
	}
	return val;
}

void Vrml97Parser::vec2fValue(
	vec2f & v
) {
#line 1944 "Vrml97Parser.g"
	
	float x, y;
	
#line 3619 "Vrml97Parser.cpp"
	
	x=floatValue();
	y=floatValue();
#line 1949 "Vrml97Parser.g"
	v.x(x);
	v.y(y);
#line 3626 "Vrml97Parser.cpp"
}

void Vrml97Parser::vec3fValue(
	vec3f & v
) {
#line 1973 "Vrml97Parser.g"
	
	float x, y, z;
	
#line 3636 "Vrml97Parser.cpp"
	
	x=floatValue();
	y=floatValue();
	z=floatValue();
#line 1978 "Vrml97Parser.g"
	v.x(x);
	v.y(y);
	v.z(z);
#line 3645 "Vrml97Parser.cpp"
}

void Vrml97Parser::initializeASTFactory( ANTLR_USE_NAMESPACE(antlr)ASTFactory& factory )
{
}
const char* Vrml97Parser::tokenNames[] = {
	"<0>",
	"EOF",
	"<2>",
	"NULL_TREE_LOOKAHEAD",
	".",
	"[",
	"]",
	"{",
	"}",
	"an identifier",
	"an integer value",
	"an integer value",
	"a floating point value",
	"a string",
	"\"DEF\"",
	"\"eventIn\"",
	"\"eventOut\"",
	"\"exposedField\"",
	"\"EXTERNPROTO\"",
	"\"FALSE\"",
	"\"field\"",
	"\"IS\"",
	"\"NULL\"",
	"\"PROTO\"",
	"\"ROUTE\"",
	"\"TO\"",
	"\"TRUE\"",
	"\"USE\"",
	"FIELDTYPE_SFBOOL",
	"FIELDTYPE_SFCOLOR",
	"FIELDTYPE_SFFLOAT",
	"FIELDTYPE_SFIMAGE",
	"FIELDTYPE_SFINT32",
	"FIELDTYPE_SFNODE",
	"FIELDTYPE_SFROTATION",
	"FIELDTYPE_SFSTRING",
	"FIELDTYPE_SFTIME",
	"FIELDTYPE_SFVEC2F",
	"FIELDTYPE_SFVEC3F",
	"FIELDTYPE_MFCOLOR",
	"FIELDTYPE_MFFLOAT",
	"FIELDTYPE_MFINT32",
	"FIELDTYPE_MFNODE",
	"FIELDTYPE_MFROTATION",
	"FIELDTYPE_MFSTRING",
	"FIELDTYPE_MFTIME",
	"FIELDTYPE_MFVEC2F",
	"FIELDTYPE_MFVEC3F",
	0
};

const unsigned long Vrml97Parser::_tokenSet_0_data_[] = { 159662592UL, 0UL, 0UL, 0UL };
// ID "DEF" "EXTERNPROTO" "PROTO" "ROUTE" "USE" 
const ANTLR_USE_NAMESPACE(antlr)BitSet Vrml97Parser::_tokenSet_0(_tokenSet_0_data_,4);
const unsigned long Vrml97Parser::_tokenSet_1_data_[] = { 25428480UL, 0UL, 0UL, 0UL };
// ID "EXTERNPROTO" "PROTO" "ROUTE" 
const ANTLR_USE_NAMESPACE(antlr)BitSet Vrml97Parser::_tokenSet_1(_tokenSet_1_data_,4);
const unsigned long Vrml97Parser::_tokenSet_2_data_[] = { 1277952UL, 0UL, 0UL, 0UL };
// "eventIn" "eventOut" "exposedField" "field" 
const ANTLR_USE_NAMESPACE(antlr)BitSet Vrml97Parser::_tokenSet_2(_tokenSet_2_data_,4);
const unsigned long Vrml97Parser::_tokenSet_3_data_[] = { 138428960UL, 0UL, 0UL, 0UL };
// LBRACKET ID "DEF" "NULL" "USE" 
const ANTLR_USE_NAMESPACE(antlr)BitSet Vrml97Parser::_tokenSet_3(_tokenSet_3_data_,4);
const unsigned long Vrml97Parser::_tokenSet_4_data_[] = { 67648544UL, 0UL, 0UL, 0UL };
// LBRACKET INTEGER HEX_INTEGER REAL STRING "FALSE" "TRUE" 
const ANTLR_USE_NAMESPACE(antlr)BitSet Vrml97Parser::_tokenSet_4(_tokenSet_4_data_,4);
const unsigned long Vrml97Parser::_tokenSet_5_data_[] = { 208174624UL, 0UL, 0UL, 0UL };
// LBRACKET ID INTEGER HEX_INTEGER REAL STRING "DEF" "FALSE" "IS" "NULL" 
// "TRUE" "USE" 
const ANTLR_USE_NAMESPACE(antlr)BitSet Vrml97Parser::_tokenSet_5(_tokenSet_5_data_,4);
const unsigned long Vrml97Parser::_tokenSet_6_data_[] = { 138428928UL, 0UL, 0UL, 0UL };
// ID "DEF" "NULL" "USE" 
const ANTLR_USE_NAMESPACE(antlr)BitSet Vrml97Parser::_tokenSet_6(_tokenSet_6_data_,4);
const unsigned long Vrml97Parser::_tokenSet_7_data_[] = { 134234656UL, 0UL, 0UL, 0UL };
// LBRACKET ID "DEF" "USE" 
const ANTLR_USE_NAMESPACE(antlr)BitSet Vrml97Parser::_tokenSet_7(_tokenSet_7_data_,4);



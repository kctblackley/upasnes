#pragma once

#include <cctype>
#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

class JsonValue {
public:
	enum class Type { Null, Bool, Number, String, Array, Object };

	Type type = Type::Null;
	bool bool_value = false;
	double number_value = 0.0;
	std::string string_value;
	std::vector<JsonValue> array_value;
	std::map<std::string, JsonValue> object_value;

	long long as_int() const { return static_cast<long long>(number_value); }
	double as_double() const { return number_value; }
	const std::string& as_string() const { return string_value; }
	bool as_bool() const { return bool_value; }

	// Object access. Missing keys return a shared Null value rather than throwing,
	// which keeps call sites simple.
	const JsonValue& operator[](const std::string& key) const {
		static const JsonValue null_value;
		auto it = object_value.find(key);
		if (it == object_value.end()) {
			return null_value;
		}
		return it->second;
	}

	// Array access.
	const JsonValue& operator[](size_t index) const {
		return array_value.at(index);
	}

	size_t size() const { return array_value.size(); }
};

class JsonParser {
public:
	explicit JsonParser(const std::string& text) : text(text), pos(0) {}

	JsonValue parse() {
		skip_whitespace();
		return parse_value();
	}

private:
	const std::string& text;
	size_t pos;

	void skip_whitespace() {
		while (pos < text.size() && std::isspace(static_cast<unsigned char>(text[pos]))) {
			pos++;
		}
	}

	char peek() const { return text[pos]; }
	char advance() { return text[pos++]; }

	JsonValue parse_value() {
		skip_whitespace();
		if (pos >= text.size()) {
			throw std::runtime_error("Unexpected end of JSON input");
		}

		char c = peek();
		if (c == '{') return parse_object();
		if (c == '[') return parse_array();
		if (c == '"') return parse_string_value();
		if (c == 't' || c == 'f') return parse_bool();
		if (c == 'n') return parse_null();
		return parse_number();
	}

	JsonValue parse_object() {
		JsonValue value;
		value.type = JsonValue::Type::Object;

		advance(); // consume '{'
		skip_whitespace();
		if (peek() == '}') {
			advance();
			return value;
		}

		while (true) {
			skip_whitespace();
			std::string key = parse_raw_string();
			skip_whitespace();
			if (advance() != ':') {
				throw std::runtime_error("Expected ':' in JSON object");
			}
			JsonValue child = parse_value();
			value.object_value.emplace(std::move(key), std::move(child));

			skip_whitespace();
			char c = advance();
			if (c == ',') continue;
			if (c == '}') break;
			throw std::runtime_error("Expected ',' or '}' in JSON object");
		}
		return value;
	}

	JsonValue parse_array() {
		JsonValue value;
		value.type = JsonValue::Type::Array;

		advance(); // consume '['
		skip_whitespace();
		if (peek() == ']') {
			advance();
			return value;
		}

		while (true) {
			value.array_value.push_back(parse_value());
			skip_whitespace();
			char c = advance();
			if (c == ',') continue;
			if (c == ']') break;
			throw std::runtime_error("Expected ',' or ']' in JSON array");
		}
		return value;
	}

	std::string parse_raw_string() {
		if (advance() != '"') {
			throw std::runtime_error("Expected string");
		}
		std::string result;
		while (true) {
			if (pos >= text.size()) {
				throw std::runtime_error("Unterminated string");
			}
			char c = advance();
			if (c == '"') break;
			if (c == '\\') {
				char escaped = advance();
				switch (escaped) {
					case 'n': result += '\n'; break;
					case 't': result += '\t'; break;
					case 'r': result += '\r'; break;
					case '"': result += '"'; break;
					case '\\': result += '\\'; break;
					case '/': result += '/'; break;
					default: result += escaped; break;
				}
			} else {
				result += c;
			}
		}
		return result;
	}

	JsonValue parse_string_value() {
		JsonValue value;
		value.type = JsonValue::Type::String;
		value.string_value = parse_raw_string();
		return value;
	}

	JsonValue parse_bool() {
		JsonValue value;
		value.type = JsonValue::Type::Bool;
		if (text.compare(pos, 4, "true") == 0) {
			value.bool_value = true;
			pos += 4;
		} else if (text.compare(pos, 5, "false") == 0) {
			value.bool_value = false;
			pos += 5;
		} else {
			throw std::runtime_error("Invalid literal in JSON");
		}
		return value;
	}

	JsonValue parse_null() {
		JsonValue value;
		value.type = JsonValue::Type::Null;
		if (text.compare(pos, 4, "null") == 0) {
			pos += 4;
		} else {
			throw std::runtime_error("Invalid literal in JSON");
		}
		return value;
	}

	JsonValue parse_number() {
		size_t start = pos;
		if (peek() == '-' || peek() == '+') advance();
		while (pos < text.size()) {
			char c = text[pos];
			bool is_number_char = std::isdigit(static_cast<unsigned char>(c)) ||
			                       c == '.' || c == 'e' || c == 'E' || c == '+' || c == '-';
			if (!is_number_char) break;
			advance();
		}
		std::string num_str = text.substr(start, pos - start);
		JsonValue value;
		value.type = JsonValue::Type::Number;
		value.number_value = std::stod(num_str);
		return value;
	}
};

// Reads and parses an entire JSON file in one go.
inline JsonValue parse_json_file(const std::string& path) {
	std::ifstream file(path);
	if (!file.is_open()) {
		throw std::runtime_error("Could not open JSON file: " + path);
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string content = buffer.str();

	JsonParser parser(content);
	return parser.parse();
}

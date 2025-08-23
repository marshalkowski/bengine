#pragma once

#include <nlohmann/json.hpp>
using json = nlohmann::json;

struct Serializable {

	virtual json to_json() {};
	virtual std::string from_json() {};
};
#pragma once
#include "databank.hpp"
#include "msgstruct.h"
#include <chrono>

int processGetRequest(msgstruct& msg, msgstruct& response, dataBank& db);

int processSetRequest(msgstruct& msg, msgstruct& response, dataBank& db);
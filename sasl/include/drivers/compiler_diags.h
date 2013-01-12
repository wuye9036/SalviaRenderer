#pragma once

#ifndef SASL_DRIVERS_COMPILER_DIAGS_H
#define SASL_DRIVERS_COMPILER_DIAGS_H

#include <sasl/include/drivers/drivers_forward.h>

#include <sasl/include/common/diag_item.h>

BEGIN_NS_SASL_DRIVERS();
extern sasl::common::diag_template text_only;
extern sasl::common::diag_template unknown_detail_level;
extern sasl::common::diag_template input_file_is_missing;
extern sasl::common::diag_template unknown_lang;
extern sasl::common::diag_template compiling_input;
END_NS_SASL_DRIVERS();

#endif
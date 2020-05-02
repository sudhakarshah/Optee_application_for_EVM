#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#include <serverForEVM_ta.h>

#define INTSIZE 64

TEE_Result TA_CreateEntryPoint(void)
{
  return TEE_SUCCESS;
}

void TA_DestroyEntryPoint(void)
{
}

TEE_Result TA_OpenSessionEntryPoint(uint32_t param_types,
    TEE_Param __maybe_unused params[4],
    void __maybe_unused **sess_ctx)
{
  uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
      TEE_PARAM_TYPE_NONE,
      TEE_PARAM_TYPE_NONE,
      TEE_PARAM_TYPE_NONE);
  if (param_types != exp_param_types)
    return TEE_ERROR_BAD_PARAMETERS;

  (void)&params;
  (void)&sess_ctx;

  return TEE_SUCCESS;
}

void TA_CloseSessionEntryPoint(void __maybe_unused *sess_ctx)
{
  (void)&sess_ctx;
}


static TEE_Result do_arithmetic(uint32_t param_types,
    TEE_Param params[4], uint32_t type)
{
  uint32_t exp_param_types =
    TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_OUTPUT,
        TEE_PARAM_TYPE_MEMREF_INPUT,
        TEE_PARAM_TYPE_MEMREF_INPUT,
        TEE_PARAM_TYPE_NONE);

  if (param_types != exp_param_types)
    return TEE_ERROR_BAD_PARAMETERS;

  uint32_t firstint[TEE_BigIntSizeInU32(INTSIZE)];
  uint32_t secondint[TEE_BigIntSizeInU32(INTSIZE)];
  uint32_t thirdint[TEE_BigIntSizeInU32(INTSIZE)];

  TEE_BigInt* first = firstint;
  TEE_BigInt* second = secondint;
  TEE_BigInt* third = thirdint;

  size_t len;
  len = (size_t) TEE_BigIntSizeInU32(INTSIZE);

  TEE_BigIntInit(first, len);
  TEE_BigIntInit(second, len);
  TEE_BigIntInit(third, len);

  TEE_BigIntConvertFromS32(first, *(uint32_t *)(params[1].memref.buffer));
  TEE_BigIntConvertFromS32(second, *(uint32_t *)(params[2].memref.buffer));

  switch(type) {
    case TA_ADD: 	
      TEE_BigIntAdd(third, first, second);
      break;
    case TA_SUBTRACT:
      TEE_BigIntSub(third, first, second);
      break;
    case TA_MULTIPLY:
      TEE_BigIntMul(third, first, second);
      break;
    default:
      return TEE_ERROR_BAD_PARAMETERS;
  }

  TEE_BigIntConvertToS32(params[0].memref.buffer, third);
  return TEE_SUCCESS;
}

TEE_Result TA_InvokeCommandEntryPoint(void __maybe_unused *sess_ctx,
    uint32_t cmd_id,
    uint32_t param_types, TEE_Param params[4])
{
  (void)&sess_ctx;

  if (cmd_id == TA_ADD || cmd_id == TA_SUBTRACT || cmd_id == TA_MULTIPLY)
    return do_arithmetic(param_types, params, cmd_id);
  else
    return TEE_ERROR_BAD_PARAMETERS;
}

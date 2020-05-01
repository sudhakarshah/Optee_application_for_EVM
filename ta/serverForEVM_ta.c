/*
 * Copyright (c) 2017, Linaro Limited
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#include <serverForEVM_ta.h>
//#include "../../../../../optee_os/lib/libmpa/include/mpa.h" 

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
	printf("DEE\n");
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

	//uint8_t os1[] = {0xFF};
	//uint8_t os2[] = {1};
	//size_t length1, length2;
	//length1 = sizeof(os1);
	//length2 = sizeof(os2);
	//TEE_BigIntConvertFromOctetString(first, os1, length1, 1);
	//TEE_BigIntConvertFromOctetString(second, os2, length2, 1);
	//printf("Octet try %d %d %d %d %d %d \n", first[0], first[1], first[2], first[3], first[4], first[5]);


	TEE_BigIntConvertFromS32(first, *(uint32_t *)(params[1].memref.buffer));
	TEE_BigIntConvertFromS32(second, *(uint32_t *)(params[2].memref.buffer));
	printf("HI\n");
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
	//printf("Printing %d %d yolo.\n", second[2], first[2]);
	
	// Basically add with same length which is max of the 2 numbers, if small, then add 1
	//uint32_t x;
	//x = TEE_BigIntCmp(first, second);
	//uint8_t os3[8] = { 0 };
	//size_t len2;
	//len2 = sizeof(os3);
	//TEE_BigIntConvertToOctetString(os3, &len2, third); 
	//printf("Compared %d\n", x);
        
	printf("Printing result %d.\n", third[2]);
	TEE_BigIntConvertToS32(params[0].memref.buffer, third);
	return TEE_SUCCESS;
}

TEE_Result TA_InvokeCommandEntryPoint(void __maybe_unused *sess_ctx,
			uint32_t cmd_id,
			uint32_t param_types, TEE_Param params[4])
{
	(void)&sess_ctx;
	printf("HELLO\n");
	if (cmd_id == TA_ADD || cmd_id == TA_SUBTRACT || cmd_id == TA_MULTIPLY)
		return do_arithmetic(param_types, params, cmd_id);
	else
		return TEE_ERROR_BAD_PARAMETERS;
}

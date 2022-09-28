#include "ccf/app_interface.h"
#include "ccf/common_auth_policies.h"
#include "executor_code_id.h"

struct ExecutorIdentity : public ccf::AuthnIdentity
{
  ExecutorId executorId;
};

class ExecutorAuthPolicy : public ccf::AuthnPolicy
{
  const ExecutorCertsMap& executor_certs = ExecutorCerts;

public:
  std::unique_ptr<ccf::AuthnIdentity> authenticate(
    kv::ReadOnlyTx&,
    const std::shared_ptr<ccf::RpcContext>& ctx,
    std::string& error_reason) override
  {
    const auto& executor_cert = ctx->get_session_context()->caller_cert;
    if (executor_cert.empty())
    {
      error_reason = "No Executor certificate";
      return nullptr;
    }

    auto executor_id = crypto::Sha256Hash(executor_cert).hex_str();

    if (executor_certs.find(executor_id) != executor_certs.end())
    {
      auto executor_identity = std::make_unique<ExecutorIdentity>();
      executor_identity->executorId = executor_id;
      return executor_identity;
    }
    error_reason = "Could not find matching Executor certificate";
    return nullptr;
  }

  std::optional<ccf::OpenAPISecuritySchema> get_openapi_security_schema()
    const override
  {
    return std::nullopt;
  }

  std::string get_security_scheme_name() override
  {
    return "ExecutorAuthPolicy";
  }
};
//
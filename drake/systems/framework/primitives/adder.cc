#include "drake/systems/framework/primitives/adder.h"

#include <cassert>
#include <stdexcept>
#include <string>

#include "drake/drakeSystemFramework_export.h"
#include "drake/systems/framework/basic_vector.h"

namespace drake {
namespace systems {

template <typename T>
Adder<T>::Adder(size_t num_inputs, size_t length)
    : num_inputs_(num_inputs), length_(length) {}

template <typename T>
std::unique_ptr<Context<T>> Adder<T>::CreateDefaultContext() const {
  std::unique_ptr<Context<T>> context(new Context<T>);
  context->get_mutable_input()->continuous_ports.resize(num_inputs_);
  return context;
}

template <typename T>
std::unique_ptr<SystemOutput<T>> Adder<T>::CreateDefaultOutput() const {
  // An adder has just one output port, a BasicVector of the size specified
  // at construction time.
  std::unique_ptr<SystemOutput<T>> output(new SystemOutput<T>);
  {
    OutputPort<T> port;
    port.output.reset(new BasicVector<T>(length_));
    output->continuous_ports.push_back(std::move(port));
  }
  return output;
}

template <typename T>
void Adder<T>::Output(const Context<T>& context,
                      SystemOutput<T>* output) const {
  // Check that the single output port has the correct length, then zero it.
  // Checks on the output structure are assertions, not exceptions,
  // since failures would reflect a bug in the Adder implementation, not
  // user error setting up the system graph. They do not require unit test
  // coverage, and should not run in release builds.
  assert(output->continuous_ports.size() == 1);
  VectorInterface<T>* output_port = output->continuous_ports[0].output.get();
  assert(output_port != nullptr);
  assert(output_port->get_value().rows() == length_);
  output_port->get_mutable_value() = VectorX<T>::Zero(length_);

  // Check that there are the expected number of input ports.
  if (context.get_input().continuous_ports.size() != num_inputs_) {
    throw std::runtime_error(
        "Expected " + std::to_string(num_inputs_) + "input ports, but had " +
        std::to_string(context.get_input().continuous_ports.size()));
  }

  // Sum each input port into the output, after checking that it has the
  // expected length.
  for (int i = 0; i < context.get_input().continuous_ports.size(); i++) {
    const VectorInterface<T>* input =
        context.get_input().continuous_ports[i].input;
    if (input == nullptr || input->get_value().rows() != length_) {
      throw std::runtime_error("Input port " + std::to_string(i) +
                               "is nullptr or has incorrect size.");
    }
    output_port->get_mutable_value() += input->get_value();
  }
}

template class DRAKESYSTEMFRAMEWORK_EXPORT Adder<double>;

}  // namespace systems
}  // namespace drake

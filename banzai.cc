#include "packet.h"
#include "pipeline.h"
#include "atom.h"
#include "dynamic_linking_loader.h"

#include <exception>
#include <vector>
#include <iostream>
#include <regex>

/// Split string based on another string used as delimiter
/// using C++11's regex_token_iterator
/// Based on http://en.cppreference.com/w/cpp/regex/regex_token_iterator
/// and http://stackoverflow.com/a/9437426/1152801
std::vector<std::string> split(const std::string & input, const std::string & regex_str) {
  std::regex regex_object(regex_str);
  std::sregex_token_iterator first{input.begin(), input.end(), regex_object, -1}, last;
  return {first, last};
}

/// Get set of strings from a comma separated value string
std::set<std::string> string_set_from_csv(const std::string & csv) {
  const std::vector<std::string> field_vector = split(csv, ",");
  return std::set<std::string>(field_vector.begin(), field_vector.end());
}

int main(const int argc __attribute__ ((unused)), const char ** argv __attribute__((unused))) {
  try {
    if (argc < 6) {
      std::cerr << "Usage: " << argv[0] << " prog_to_run_as_library seed comma_separated_input_field_list comma_seperated_output_field_set num_ticks" << std::endl;
      return EXIT_FAILURE;
    }

    // Get cmdline args
    const auto prog_to_run(argv[1]);
    const uint32_t seed = static_cast<uint32_t>(std::atoi(argv[2]));
    const PacketFieldSet input_field_set  = string_set_from_csv(std::string(argv[3]));
    const PacketFieldSet output_field_set = string_set_from_csv(std::string(argv[4]));
    const uint32_t num_ticks = static_cast<uint32_t>(std::atoi(argv[5]));

    /// PRNG to generate random packet fields,
    std::default_random_engine prng = std::default_random_engine(seed);

    /// Uniform distribution over ints to generate random packet fields
    std::uniform_int_distribution<int> packet_field_dist(std::numeric_limits<int>::lowest(), std::numeric_limits<int>::max());

    // Construct shared library loader for prog_to_run
    DynamicLinkingLoader dynamic_linking_loader(prog_to_run);
    Pipeline  pipeline = dynamic_linking_loader.get_object<Pipeline>("test_pipeline");

    // Run for num_ticks
    for (uint32_t i = 0; i < num_ticks; i++) {
      // Generate random input packets using packet_field_set
      // Construct Packet using an empty FieldContainer to signal that this isn't
      // a default-constructed packet, which is treated as a bubble.
      auto input_packet = Packet(FieldContainer<int>());
      for (const auto & field_name : input_field_set) input_packet(field_name) = packet_field_dist(prng);

      // Print out user-specified fields in the output_packet
      auto output_packet = pipeline.tick(input_packet);
      if (not output_packet.is_bubble()) for (const auto & field_name : output_field_set) std::cerr << field_name << " " << output_packet(field_name) << std::endl;
    }
    return EXIT_SUCCESS;
  } catch (const std::exception & e) {
    std::cerr << "Caught exception in main " << std::endl << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}

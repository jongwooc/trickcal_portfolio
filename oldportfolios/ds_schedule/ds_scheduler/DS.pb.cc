// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: DS.proto

#include "DS.pb.h"

#include <algorithm>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/port.h>
#include <google/protobuf/stubs/once.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// This is a temporary google only hack
#ifdef GOOGLE_PROTOBUF_ENFORCE_UNIQUENESS
#include "third_party/protobuf/version.h"
#endif
// @@protoc_insertion_point(includes)
namespace DS {
class network_LayerDefaultTypeInternal {
 public:
  ::google::protobuf::internal::ExplicitlyConstructed<network_Layer>
      _instance;
} _network_Layer_default_instance_;
class networkDefaultTypeInternal {
 public:
  ::google::protobuf::internal::ExplicitlyConstructed<network>
      _instance;
} _network_default_instance_;
}  // namespace DS
namespace protobuf_DS_2eproto {
void InitDefaultsnetwork_LayerImpl() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

#ifdef GOOGLE_PROTOBUF_ENFORCE_UNIQUENESS
  ::google::protobuf::internal::InitProtobufDefaultsForceUnique();
#else
  ::google::protobuf::internal::InitProtobufDefaults();
#endif  // GOOGLE_PROTOBUF_ENFORCE_UNIQUENESS
  {
    void* ptr = &::DS::_network_Layer_default_instance_;
    new (ptr) ::DS::network_Layer();
    ::google::protobuf::internal::OnShutdownDestroyMessage(ptr);
  }
  ::DS::network_Layer::InitAsDefaultInstance();
}

void InitDefaultsnetwork_Layer() {
  static GOOGLE_PROTOBUF_DECLARE_ONCE(once);
  ::google::protobuf::GoogleOnceInit(&once, &InitDefaultsnetwork_LayerImpl);
}

void InitDefaultsnetworkImpl() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

#ifdef GOOGLE_PROTOBUF_ENFORCE_UNIQUENESS
  ::google::protobuf::internal::InitProtobufDefaultsForceUnique();
#else
  ::google::protobuf::internal::InitProtobufDefaults();
#endif  // GOOGLE_PROTOBUF_ENFORCE_UNIQUENESS
  protobuf_DS_2eproto::InitDefaultsnetwork_Layer();
  {
    void* ptr = &::DS::_network_default_instance_;
    new (ptr) ::DS::network();
    ::google::protobuf::internal::OnShutdownDestroyMessage(ptr);
  }
  ::DS::network::InitAsDefaultInstance();
}

void InitDefaultsnetwork() {
  static GOOGLE_PROTOBUF_DECLARE_ONCE(once);
  ::google::protobuf::GoogleOnceInit(&once, &InitDefaultsnetworkImpl);
}

::google::protobuf::Metadata file_level_metadata[2];

const ::google::protobuf::uint32 TableStruct::offsets[] GOOGLE_PROTOBUF_ATTRIBUTE_SECTION_VARIABLE(protodesc_cold) = {
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::DS::network_Layer, _has_bits_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::DS::network_Layer, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::DS::network_Layer, name_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::DS::network_Layer, time_unit_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::DS::network_Layer, cpu_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::DS::network_Layer, gpu_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::DS::network_Layer, dsp_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::DS::network_Layer, npu_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::DS::network_Layer, cpu2_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::DS::network_Layer, cpu3_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::DS::network_Layer, cpu4_),
  0,
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::DS::network, _has_bits_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::DS::network, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::DS::network, name_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::DS::network, layer_),
  0,
  ~0u,
};
static const ::google::protobuf::internal::MigrationSchema schemas[] GOOGLE_PROTOBUF_ATTRIBUTE_SECTION_VARIABLE(protodesc_cold) = {
  { 0, 14, sizeof(::DS::network_Layer)},
  { 23, 30, sizeof(::DS::network)},
};

static ::google::protobuf::Message const * const file_default_instances[] = {
  reinterpret_cast<const ::google::protobuf::Message*>(&::DS::_network_Layer_default_instance_),
  reinterpret_cast<const ::google::protobuf::Message*>(&::DS::_network_default_instance_),
};

void protobuf_AssignDescriptors() {
  AddDescriptors();
  ::google::protobuf::MessageFactory* factory = NULL;
  AssignDescriptors(
      "DS.proto", schemas, file_default_instances, TableStruct::offsets, factory,
      file_level_metadata, NULL, NULL);
}

void protobuf_AssignDescriptorsOnce() {
  static GOOGLE_PROTOBUF_DECLARE_ONCE(once);
  ::google::protobuf::GoogleOnceInit(&once, &protobuf_AssignDescriptors);
}

void protobuf_RegisterTypes(const ::std::string&) GOOGLE_PROTOBUF_ATTRIBUTE_COLD;
void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::internal::RegisterAllTypes(file_level_metadata, 2);
}

void AddDescriptorsImpl() {
  InitDefaults();
  static const char descriptor[] GOOGLE_PROTOBUF_ATTRIBUTE_SECTION_VARIABLE(protodesc_cold) = {
      "\n\010DS.proto\022\002DS\"\302\001\n\007network\022\014\n\004name\030\001 \001(\t"
      "\022 \n\005layer\030\002 \003(\0132\021.DS.network.Layer\032\206\001\n\005L"
      "ayer\022\014\n\004name\030\001 \001(\t\022\021\n\ttime_unit\030\002 \001(\t\022\013\n"
      "\003cpu\030\003 \001(\002\022\013\n\003gpu\030\004 \001(\002\022\013\n\003dsp\030\005 \001(\002\022\013\n\003"
      "npu\030\006 \001(\002\022\014\n\004cpu2\030\007 \001(\002\022\014\n\004cpu3\030\010 \001(\002\022\014\n"
      "\004cpu4\030\t \001(\002"
  };
  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
      descriptor, 211);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "DS.proto", &protobuf_RegisterTypes);
}

void AddDescriptors() {
  static GOOGLE_PROTOBUF_DECLARE_ONCE(once);
  ::google::protobuf::GoogleOnceInit(&once, &AddDescriptorsImpl);
}
// Force AddDescriptors() to be called at dynamic initialization time.
struct StaticDescriptorInitializer {
  StaticDescriptorInitializer() {
    AddDescriptors();
  }
} static_descriptor_initializer;
}  // namespace protobuf_DS_2eproto
namespace DS {

// ===================================================================

void network_Layer::InitAsDefaultInstance() {
}
#if !defined(_MSC_VER) || _MSC_VER >= 1900
const int network_Layer::kNameFieldNumber;
const int network_Layer::kTimeUnitFieldNumber;
const int network_Layer::kCpuFieldNumber;
const int network_Layer::kGpuFieldNumber;
const int network_Layer::kDspFieldNumber;
const int network_Layer::kNpuFieldNumber;
const int network_Layer::kCpu2FieldNumber;
const int network_Layer::kCpu3FieldNumber;
const int network_Layer::kCpu4FieldNumber;
#endif  // !defined(_MSC_VER) || _MSC_VER >= 1900

network_Layer::network_Layer()
  : ::google::protobuf::Message(), _internal_metadata_(NULL) {
  if (GOOGLE_PREDICT_TRUE(this != internal_default_instance())) {
    ::protobuf_DS_2eproto::InitDefaultsnetwork_Layer();
  }
  SharedCtor();
  // @@protoc_insertion_point(constructor:DS.network.Layer)
}
network_Layer::network_Layer(const network_Layer& from)
  : ::google::protobuf::Message(),
      _internal_metadata_(NULL),
      _has_bits_(from._has_bits_),
      _cached_size_(0) {
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  name_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  if (from.has_name()) {
    name_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.name_);
  }
  time_unit_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  if (from.has_time_unit()) {
    time_unit_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.time_unit_);
  }
  ::memcpy(&cpu_, &from.cpu_,
    static_cast<size_t>(reinterpret_cast<char*>(&cpu4_) -
    reinterpret_cast<char*>(&cpu_)) + sizeof(cpu4_));
  // @@protoc_insertion_point(copy_constructor:DS.network.Layer)
}

void network_Layer::SharedCtor() {
  _cached_size_ = 0;
  name_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  time_unit_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  ::memset(&cpu_, 0, static_cast<size_t>(
      reinterpret_cast<char*>(&cpu4_) -
      reinterpret_cast<char*>(&cpu_)) + sizeof(cpu4_));
}

network_Layer::~network_Layer() {
  // @@protoc_insertion_point(destructor:DS.network.Layer)
  SharedDtor();
}

void network_Layer::SharedDtor() {
  name_.DestroyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  time_unit_.DestroyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}

void network_Layer::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* network_Layer::descriptor() {
  ::protobuf_DS_2eproto::protobuf_AssignDescriptorsOnce();
  return ::protobuf_DS_2eproto::file_level_metadata[kIndexInFileMessages].descriptor;
}

const network_Layer& network_Layer::default_instance() {
  ::protobuf_DS_2eproto::InitDefaultsnetwork_Layer();
  return *internal_default_instance();
}

network_Layer* network_Layer::New(::google::protobuf::Arena* arena) const {
  network_Layer* n = new network_Layer;
  if (arena != NULL) {
    arena->Own(n);
  }
  return n;
}

void network_Layer::Clear() {
// @@protoc_insertion_point(message_clear_start:DS.network.Layer)
  ::google::protobuf::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  cached_has_bits = _has_bits_[0];
  if (cached_has_bits & 3u) {
    if (cached_has_bits & 0x00000001u) {
      GOOGLE_DCHECK(!name_.IsDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited()));
      (*name_.UnsafeRawStringPointer())->clear();
    }
    if (cached_has_bits & 0x00000002u) {
      GOOGLE_DCHECK(!time_unit_.IsDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited()));
      (*time_unit_.UnsafeRawStringPointer())->clear();
    }
  }
  if (cached_has_bits & 252u) {
    ::memset(&cpu_, 0, static_cast<size_t>(
        reinterpret_cast<char*>(&cpu3_) -
        reinterpret_cast<char*>(&cpu_)) + sizeof(cpu3_));
  }
  cpu4_ = 0;
  _has_bits_.Clear();
  _internal_metadata_.Clear();
}

bool network_Layer::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!GOOGLE_PREDICT_TRUE(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:DS.network.Layer)
  for (;;) {
    ::std::pair< ::google::protobuf::uint32, bool> p = input->ReadTagWithCutoffNoLastTag(127u);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // optional string name = 1;
      case 1: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(10u /* 10 & 0xFF */)) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_name()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
            this->name().data(), static_cast<int>(this->name().length()),
            ::google::protobuf::internal::WireFormat::PARSE,
            "DS.network.Layer.name");
        } else {
          goto handle_unusual;
        }
        break;
      }

      // optional string time_unit = 2;
      case 2: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(18u /* 18 & 0xFF */)) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_time_unit()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
            this->time_unit().data(), static_cast<int>(this->time_unit().length()),
            ::google::protobuf::internal::WireFormat::PARSE,
            "DS.network.Layer.time_unit");
        } else {
          goto handle_unusual;
        }
        break;
      }

      // optional float cpu = 3;
      case 3: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(29u /* 29 & 0xFF */)) {
          set_has_cpu();
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   float, ::google::protobuf::internal::WireFormatLite::TYPE_FLOAT>(
                 input, &cpu_)));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // optional float gpu = 4;
      case 4: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(37u /* 37 & 0xFF */)) {
          set_has_gpu();
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   float, ::google::protobuf::internal::WireFormatLite::TYPE_FLOAT>(
                 input, &gpu_)));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // optional float dsp = 5;
      case 5: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(45u /* 45 & 0xFF */)) {
          set_has_dsp();
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   float, ::google::protobuf::internal::WireFormatLite::TYPE_FLOAT>(
                 input, &dsp_)));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // optional float npu = 6;
      case 6: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(53u /* 53 & 0xFF */)) {
          set_has_npu();
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   float, ::google::protobuf::internal::WireFormatLite::TYPE_FLOAT>(
                 input, &npu_)));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // optional float cpu2 = 7;
      case 7: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(61u /* 61 & 0xFF */)) {
          set_has_cpu2();
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   float, ::google::protobuf::internal::WireFormatLite::TYPE_FLOAT>(
                 input, &cpu2_)));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // optional float cpu3 = 8;
      case 8: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(69u /* 69 & 0xFF */)) {
          set_has_cpu3();
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   float, ::google::protobuf::internal::WireFormatLite::TYPE_FLOAT>(
                 input, &cpu3_)));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // optional float cpu4 = 9;
      case 9: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(77u /* 77 & 0xFF */)) {
          set_has_cpu4();
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   float, ::google::protobuf::internal::WireFormatLite::TYPE_FLOAT>(
                 input, &cpu4_)));
        } else {
          goto handle_unusual;
        }
        break;
      }

      default: {
      handle_unusual:
        if (tag == 0) {
          goto success;
        }
        DO_(::google::protobuf::internal::WireFormat::SkipField(
              input, tag, _internal_metadata_.mutable_unknown_fields()));
        break;
      }
    }
  }
success:
  // @@protoc_insertion_point(parse_success:DS.network.Layer)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:DS.network.Layer)
  return false;
#undef DO_
}

void network_Layer::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:DS.network.Layer)
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  cached_has_bits = _has_bits_[0];
  // optional string name = 1;
  if (cached_has_bits & 0x00000001u) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->name().data(), static_cast<int>(this->name().length()),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "DS.network.Layer.name");
    ::google::protobuf::internal::WireFormatLite::WriteStringMaybeAliased(
      1, this->name(), output);
  }

  // optional string time_unit = 2;
  if (cached_has_bits & 0x00000002u) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->time_unit().data(), static_cast<int>(this->time_unit().length()),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "DS.network.Layer.time_unit");
    ::google::protobuf::internal::WireFormatLite::WriteStringMaybeAliased(
      2, this->time_unit(), output);
  }

  // optional float cpu = 3;
  if (cached_has_bits & 0x00000004u) {
    ::google::protobuf::internal::WireFormatLite::WriteFloat(3, this->cpu(), output);
  }

  // optional float gpu = 4;
  if (cached_has_bits & 0x00000008u) {
    ::google::protobuf::internal::WireFormatLite::WriteFloat(4, this->gpu(), output);
  }

  // optional float dsp = 5;
  if (cached_has_bits & 0x00000010u) {
    ::google::protobuf::internal::WireFormatLite::WriteFloat(5, this->dsp(), output);
  }

  // optional float npu = 6;
  if (cached_has_bits & 0x00000020u) {
    ::google::protobuf::internal::WireFormatLite::WriteFloat(6, this->npu(), output);
  }

  // optional float cpu2 = 7;
  if (cached_has_bits & 0x00000040u) {
    ::google::protobuf::internal::WireFormatLite::WriteFloat(7, this->cpu2(), output);
  }

  // optional float cpu3 = 8;
  if (cached_has_bits & 0x00000080u) {
    ::google::protobuf::internal::WireFormatLite::WriteFloat(8, this->cpu3(), output);
  }

  // optional float cpu4 = 9;
  if (cached_has_bits & 0x00000100u) {
    ::google::protobuf::internal::WireFormatLite::WriteFloat(9, this->cpu4(), output);
  }

  if (_internal_metadata_.have_unknown_fields()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        _internal_metadata_.unknown_fields(), output);
  }
  // @@protoc_insertion_point(serialize_end:DS.network.Layer)
}

::google::protobuf::uint8* network_Layer::InternalSerializeWithCachedSizesToArray(
    bool deterministic, ::google::protobuf::uint8* target) const {
  (void)deterministic; // Unused
  // @@protoc_insertion_point(serialize_to_array_start:DS.network.Layer)
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  cached_has_bits = _has_bits_[0];
  // optional string name = 1;
  if (cached_has_bits & 0x00000001u) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->name().data(), static_cast<int>(this->name().length()),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "DS.network.Layer.name");
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        1, this->name(), target);
  }

  // optional string time_unit = 2;
  if (cached_has_bits & 0x00000002u) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->time_unit().data(), static_cast<int>(this->time_unit().length()),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "DS.network.Layer.time_unit");
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        2, this->time_unit(), target);
  }

  // optional float cpu = 3;
  if (cached_has_bits & 0x00000004u) {
    target = ::google::protobuf::internal::WireFormatLite::WriteFloatToArray(3, this->cpu(), target);
  }

  // optional float gpu = 4;
  if (cached_has_bits & 0x00000008u) {
    target = ::google::protobuf::internal::WireFormatLite::WriteFloatToArray(4, this->gpu(), target);
  }

  // optional float dsp = 5;
  if (cached_has_bits & 0x00000010u) {
    target = ::google::protobuf::internal::WireFormatLite::WriteFloatToArray(5, this->dsp(), target);
  }

  // optional float npu = 6;
  if (cached_has_bits & 0x00000020u) {
    target = ::google::protobuf::internal::WireFormatLite::WriteFloatToArray(6, this->npu(), target);
  }

  // optional float cpu2 = 7;
  if (cached_has_bits & 0x00000040u) {
    target = ::google::protobuf::internal::WireFormatLite::WriteFloatToArray(7, this->cpu2(), target);
  }

  // optional float cpu3 = 8;
  if (cached_has_bits & 0x00000080u) {
    target = ::google::protobuf::internal::WireFormatLite::WriteFloatToArray(8, this->cpu3(), target);
  }

  // optional float cpu4 = 9;
  if (cached_has_bits & 0x00000100u) {
    target = ::google::protobuf::internal::WireFormatLite::WriteFloatToArray(9, this->cpu4(), target);
  }

  if (_internal_metadata_.have_unknown_fields()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields(), target);
  }
  // @@protoc_insertion_point(serialize_to_array_end:DS.network.Layer)
  return target;
}

size_t network_Layer::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:DS.network.Layer)
  size_t total_size = 0;

  if (_internal_metadata_.have_unknown_fields()) {
    total_size +=
      ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        _internal_metadata_.unknown_fields());
  }
  if (_has_bits_[0 / 32] & 255u) {
    // optional string name = 1;
    if (has_name()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->name());
    }

    // optional string time_unit = 2;
    if (has_time_unit()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->time_unit());
    }

    // optional float cpu = 3;
    if (has_cpu()) {
      total_size += 1 + 4;
    }

    // optional float gpu = 4;
    if (has_gpu()) {
      total_size += 1 + 4;
    }

    // optional float dsp = 5;
    if (has_dsp()) {
      total_size += 1 + 4;
    }

    // optional float npu = 6;
    if (has_npu()) {
      total_size += 1 + 4;
    }

    // optional float cpu2 = 7;
    if (has_cpu2()) {
      total_size += 1 + 4;
    }

    // optional float cpu3 = 8;
    if (has_cpu3()) {
      total_size += 1 + 4;
    }

  }
  // optional float cpu4 = 9;
  if (has_cpu4()) {
    total_size += 1 + 4;
  }

  int cached_size = ::google::protobuf::internal::ToCachedSize(total_size);
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = cached_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void network_Layer::MergeFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_merge_from_start:DS.network.Layer)
  GOOGLE_DCHECK_NE(&from, this);
  const network_Layer* source =
      ::google::protobuf::internal::DynamicCastToGenerated<const network_Layer>(
          &from);
  if (source == NULL) {
  // @@protoc_insertion_point(generalized_merge_from_cast_fail:DS.network.Layer)
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
  // @@protoc_insertion_point(generalized_merge_from_cast_success:DS.network.Layer)
    MergeFrom(*source);
  }
}

void network_Layer::MergeFrom(const network_Layer& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:DS.network.Layer)
  GOOGLE_DCHECK_NE(&from, this);
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  cached_has_bits = from._has_bits_[0];
  if (cached_has_bits & 255u) {
    if (cached_has_bits & 0x00000001u) {
      set_has_name();
      name_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.name_);
    }
    if (cached_has_bits & 0x00000002u) {
      set_has_time_unit();
      time_unit_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.time_unit_);
    }
    if (cached_has_bits & 0x00000004u) {
      cpu_ = from.cpu_;
    }
    if (cached_has_bits & 0x00000008u) {
      gpu_ = from.gpu_;
    }
    if (cached_has_bits & 0x00000010u) {
      dsp_ = from.dsp_;
    }
    if (cached_has_bits & 0x00000020u) {
      npu_ = from.npu_;
    }
    if (cached_has_bits & 0x00000040u) {
      cpu2_ = from.cpu2_;
    }
    if (cached_has_bits & 0x00000080u) {
      cpu3_ = from.cpu3_;
    }
    _has_bits_[0] |= cached_has_bits;
  }
  if (cached_has_bits & 0x00000100u) {
    set_cpu4(from.cpu4());
  }
}

void network_Layer::CopyFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_copy_from_start:DS.network.Layer)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void network_Layer::CopyFrom(const network_Layer& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:DS.network.Layer)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool network_Layer::IsInitialized() const {
  return true;
}

void network_Layer::Swap(network_Layer* other) {
  if (other == this) return;
  InternalSwap(other);
}
void network_Layer::InternalSwap(network_Layer* other) {
  using std::swap;
  name_.Swap(&other->name_);
  time_unit_.Swap(&other->time_unit_);
  swap(cpu_, other->cpu_);
  swap(gpu_, other->gpu_);
  swap(dsp_, other->dsp_);
  swap(npu_, other->npu_);
  swap(cpu2_, other->cpu2_);
  swap(cpu3_, other->cpu3_);
  swap(cpu4_, other->cpu4_);
  swap(_has_bits_[0], other->_has_bits_[0]);
  _internal_metadata_.Swap(&other->_internal_metadata_);
  swap(_cached_size_, other->_cached_size_);
}

::google::protobuf::Metadata network_Layer::GetMetadata() const {
  protobuf_DS_2eproto::protobuf_AssignDescriptorsOnce();
  return ::protobuf_DS_2eproto::file_level_metadata[kIndexInFileMessages];
}


// ===================================================================

void network::InitAsDefaultInstance() {
}
#if !defined(_MSC_VER) || _MSC_VER >= 1900
const int network::kNameFieldNumber;
const int network::kLayerFieldNumber;
#endif  // !defined(_MSC_VER) || _MSC_VER >= 1900

network::network()
  : ::google::protobuf::Message(), _internal_metadata_(NULL) {
  if (GOOGLE_PREDICT_TRUE(this != internal_default_instance())) {
    ::protobuf_DS_2eproto::InitDefaultsnetwork();
  }
  SharedCtor();
  // @@protoc_insertion_point(constructor:DS.network)
}
network::network(const network& from)
  : ::google::protobuf::Message(),
      _internal_metadata_(NULL),
      _has_bits_(from._has_bits_),
      _cached_size_(0),
      layer_(from.layer_) {
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  name_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  if (from.has_name()) {
    name_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.name_);
  }
  // @@protoc_insertion_point(copy_constructor:DS.network)
}

void network::SharedCtor() {
  _cached_size_ = 0;
  name_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}

network::~network() {
  // @@protoc_insertion_point(destructor:DS.network)
  SharedDtor();
}

void network::SharedDtor() {
  name_.DestroyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}

void network::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* network::descriptor() {
  ::protobuf_DS_2eproto::protobuf_AssignDescriptorsOnce();
  return ::protobuf_DS_2eproto::file_level_metadata[kIndexInFileMessages].descriptor;
}

const network& network::default_instance() {
  ::protobuf_DS_2eproto::InitDefaultsnetwork();
  return *internal_default_instance();
}

network* network::New(::google::protobuf::Arena* arena) const {
  network* n = new network;
  if (arena != NULL) {
    arena->Own(n);
  }
  return n;
}

void network::Clear() {
// @@protoc_insertion_point(message_clear_start:DS.network)
  ::google::protobuf::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  layer_.Clear();
  cached_has_bits = _has_bits_[0];
  if (cached_has_bits & 0x00000001u) {
    GOOGLE_DCHECK(!name_.IsDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited()));
    (*name_.UnsafeRawStringPointer())->clear();
  }
  _has_bits_.Clear();
  _internal_metadata_.Clear();
}

bool network::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!GOOGLE_PREDICT_TRUE(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:DS.network)
  for (;;) {
    ::std::pair< ::google::protobuf::uint32, bool> p = input->ReadTagWithCutoffNoLastTag(127u);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // optional string name = 1;
      case 1: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(10u /* 10 & 0xFF */)) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_name()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
            this->name().data(), static_cast<int>(this->name().length()),
            ::google::protobuf::internal::WireFormat::PARSE,
            "DS.network.name");
        } else {
          goto handle_unusual;
        }
        break;
      }

      // repeated .DS.network.Layer layer = 2;
      case 2: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(18u /* 18 & 0xFF */)) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadMessage(input, add_layer()));
        } else {
          goto handle_unusual;
        }
        break;
      }

      default: {
      handle_unusual:
        if (tag == 0) {
          goto success;
        }
        DO_(::google::protobuf::internal::WireFormat::SkipField(
              input, tag, _internal_metadata_.mutable_unknown_fields()));
        break;
      }
    }
  }
success:
  // @@protoc_insertion_point(parse_success:DS.network)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:DS.network)
  return false;
#undef DO_
}

void network::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:DS.network)
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  cached_has_bits = _has_bits_[0];
  // optional string name = 1;
  if (cached_has_bits & 0x00000001u) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->name().data(), static_cast<int>(this->name().length()),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "DS.network.name");
    ::google::protobuf::internal::WireFormatLite::WriteStringMaybeAliased(
      1, this->name(), output);
  }

  // repeated .DS.network.Layer layer = 2;
  for (unsigned int i = 0,
      n = static_cast<unsigned int>(this->layer_size()); i < n; i++) {
    ::google::protobuf::internal::WireFormatLite::WriteMessageMaybeToArray(
      2, this->layer(static_cast<int>(i)), output);
  }

  if (_internal_metadata_.have_unknown_fields()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        _internal_metadata_.unknown_fields(), output);
  }
  // @@protoc_insertion_point(serialize_end:DS.network)
}

::google::protobuf::uint8* network::InternalSerializeWithCachedSizesToArray(
    bool deterministic, ::google::protobuf::uint8* target) const {
  (void)deterministic; // Unused
  // @@protoc_insertion_point(serialize_to_array_start:DS.network)
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  cached_has_bits = _has_bits_[0];
  // optional string name = 1;
  if (cached_has_bits & 0x00000001u) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->name().data(), static_cast<int>(this->name().length()),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "DS.network.name");
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        1, this->name(), target);
  }

  // repeated .DS.network.Layer layer = 2;
  for (unsigned int i = 0,
      n = static_cast<unsigned int>(this->layer_size()); i < n; i++) {
    target = ::google::protobuf::internal::WireFormatLite::
      InternalWriteMessageToArray(
        2, this->layer(static_cast<int>(i)), deterministic, target);
  }

  if (_internal_metadata_.have_unknown_fields()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields(), target);
  }
  // @@protoc_insertion_point(serialize_to_array_end:DS.network)
  return target;
}

size_t network::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:DS.network)
  size_t total_size = 0;

  if (_internal_metadata_.have_unknown_fields()) {
    total_size +=
      ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        _internal_metadata_.unknown_fields());
  }
  // repeated .DS.network.Layer layer = 2;
  {
    unsigned int count = static_cast<unsigned int>(this->layer_size());
    total_size += 1UL * count;
    for (unsigned int i = 0; i < count; i++) {
      total_size +=
        ::google::protobuf::internal::WireFormatLite::MessageSize(
          this->layer(static_cast<int>(i)));
    }
  }

  // optional string name = 1;
  if (has_name()) {
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::StringSize(
        this->name());
  }

  int cached_size = ::google::protobuf::internal::ToCachedSize(total_size);
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = cached_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void network::MergeFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_merge_from_start:DS.network)
  GOOGLE_DCHECK_NE(&from, this);
  const network* source =
      ::google::protobuf::internal::DynamicCastToGenerated<const network>(
          &from);
  if (source == NULL) {
  // @@protoc_insertion_point(generalized_merge_from_cast_fail:DS.network)
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
  // @@protoc_insertion_point(generalized_merge_from_cast_success:DS.network)
    MergeFrom(*source);
  }
}

void network::MergeFrom(const network& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:DS.network)
  GOOGLE_DCHECK_NE(&from, this);
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  layer_.MergeFrom(from.layer_);
  if (from.has_name()) {
    set_has_name();
    name_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.name_);
  }
}

void network::CopyFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_copy_from_start:DS.network)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void network::CopyFrom(const network& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:DS.network)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool network::IsInitialized() const {
  return true;
}

void network::Swap(network* other) {
  if (other == this) return;
  InternalSwap(other);
}
void network::InternalSwap(network* other) {
  using std::swap;
  layer_.InternalSwap(&other->layer_);
  name_.Swap(&other->name_);
  swap(_has_bits_[0], other->_has_bits_[0]);
  _internal_metadata_.Swap(&other->_internal_metadata_);
  swap(_cached_size_, other->_cached_size_);
}

::google::protobuf::Metadata network::GetMetadata() const {
  protobuf_DS_2eproto::protobuf_AssignDescriptorsOnce();
  return ::protobuf_DS_2eproto::file_level_metadata[kIndexInFileMessages];
}


// @@protoc_insertion_point(namespace_scope)
}  // namespace DS

// @@protoc_insertion_point(global_scope)

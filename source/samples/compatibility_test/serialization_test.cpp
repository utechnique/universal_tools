//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "serialization_test.h"
//----------------------------------------------------------------------------//
// simple polymorphic types
UT_REGISTER_TYPE(TestBase, TestBase, "test_base")
UT_REGISTER_TYPE(TestBase, PolymorphicA, "test_derived_A")
UT_REGISTER_TYPE(TestBase, PolymorphicB, "test_derived_B")

// polymorphic reflective types
UT_REGISTER_TYPE(ReflectiveBase, ReflectiveBase, "reflective_base")
UT_REGISTER_TYPE(ReflectiveBase, ReflectiveA, "reflective_A")
UT_REGISTER_TYPE(ReflectiveBase, ReflectiveB, "reflective_B")

// alternate reflective types, but with the same names
// these types are used to test versioning problems via serialization:
// alternate types have another order of the parameters and several
// excessive / missing parameters
UT_REGISTER_TYPE(ReflectiveBaseAlt, ReflectiveBaseAlt, "reflective_base")
UT_REGISTER_TYPE(ReflectiveBaseAlt, ReflectiveAltB, "reflective_B")

//----------------------------------------------------------------------------//
SerializationTestUnit::SerializationTestUnit() : TestUnit("SERIALIZATION")
{
	tasks.Add(new SerializationTask);
	tasks.Add(new TextSerializationTask);
}

//----------------------------------------------------------------------------//
SerializationTask::SerializationTask() : TestTask("Binary serialization")
{ }

void SerializationTask::Execute()
{
	// create and change the object
	SerializationTest object0(false);
	ChangeSerializedObject(object0);

	// save changed object to the stream
	ut::BinaryStream stream;
	ut::Optional<ut::Error> save_error = object0.Save(stream);
	if (save_error)
	{
		report += "Failed to save binary object:\n";
		report += save_error.Get().GetDesc();
		failed_test_counter.Increment();
		return;
	}

	// move stream cursor back
	stream.MoveCursor(0);

	// load another object from the stream, it must be
	// the same as the original one
	SerializationTest object1(true);
	ut::Optional<ut::Error> load_error = object1.Load(stream);
	if (load_error)
	{
		report += "Failed to load binary object:\n";
		report += load_error.Get().GetDesc();
		failed_test_counter.Increment();
		return;
	}

	// validate object immutability after save/load action
	if (CheckSerializedObject(object1))
	{
		report += "Success: Objects match after serialization/deserialization. ";
	}
	else
	{
		report += "FAIL: Objects don't match after serialization/deserialization. ";
		failed_test_counter.Increment();
	}
}

//----------------------------------------------------------------------------//
TextSerializationTask::TextSerializationTask() : TestTask("Text serialization")
{ }

void TextSerializationTask::Execute()
{
	// create and change the object
	SerializationTest object0(false);
	ChangeSerializedObject(object0);

	// you can save serialized object to .xml file here
	bool save_to_file = false;
	if (save_to_file)
	{
		ut::File xml_file;
		ut::File json_file;
		ut::File binary_file;
		const ut::String xml_filename = "serialization.xml";
		const ut::String json_filename = "serialization.json";
		const ut::String binary_filename = "serialization.bin";

		report += "Saving xml file: ";
		ut::Optional<ut::Error> open_xml_error = xml_file.Open(xml_filename, ut::file_access_write);
		if (open_xml_error)
		{
			report += "failed. ";
			report += open_xml_error.Get().GetDesc();
			failed_test_counter.Increment();
		}
		else
		{
			ut::XmlDoc xml_doc;
			xml_file << (xml_doc << object0);
			report += "success. ";
		}
		xml_file.Close();

		report += "Saving json file: ";
		ut::Optional<ut::Error> open_json_error = json_file.Open(json_filename, ut::file_access_write);
		if (open_json_error)
		{
			report += "failed. ";
			report += open_json_error.Get().GetDesc();
			failed_test_counter.Increment();
		}
		else
		{
			ut::JsonDoc json_doc;
			json_file << (json_doc << object0);
			report += "success. ";
		}
		json_file.Close();

		report += "Saving binary file: ";
		ut::Optional<ut::Error> open_binary_error = binary_file.Open(binary_filename, ut::file_access_write);
		if (open_binary_error)
		{
			report += "failed. ";
			report += open_binary_error.Get().GetDesc();
			failed_test_counter.Increment();
		}
		else
		{
			object0.Save(binary_file);
			report += "success. ";
		}
		binary_file.Close();
	}

	// xml serialization
	ut::BinaryStream xml_stream;
	ut::XmlDoc save_xml;
	xml_stream << (save_xml << object0);
	xml_stream.MoveCursor(0);

	// json serialization
	ut::BinaryStream json_stream;
	ut::JsonDoc save_json;
	json_stream << (save_json << object0);
	json_stream.MoveCursor(0);

	// load another object from the stream, it must be
	// the same as the original one
	SerializationTest xml_object(true);
	ut::XmlDoc load_xml;
	xml_stream >> load_xml >> xml_object;
	SerializationTest json_object(true);
	ut::JsonDoc load_json;
	json_stream >> load_json >> json_object;

	// validate xml object immutability after save/load action
	if (CheckSerializedObject(xml_object))
	{
		report += "XML file was read successfully: objects match after serialization/deserialization. ";
	}
	else
	{
		report += "FAIL: Objects don't match after XML serialization/deserialization. ";
		failed_test_counter.Increment();
	}

	// validate json object immutability after save/load action
	if (CheckSerializedObject(json_object))
	{
		report += "JSON file was read successfully: objects match after serialization/deserialization. ";
	}
	else
	{
		report += "JSON: Objects don't match after XML serialization/deserialization. ";
		failed_test_counter.Increment();
	}
}

//----------------------------------------------------------------------------//
SerializationSubClass::SerializationSubClass() : u16val(0), str("void")
{ }

void SerializationSubClass::Serialize(ut::MetaStream& stream)
{
	stream << u16val;
	stream << iarr;
	stream << str;
}

//----------------------------------------------------------------------------//
SerializationTest::SerializationTest(bool in_alternate) : alternate(in_alternate)
                                                        , ival(0)
                                                        , uval(0)
                                                        , fval(0.0f)
                                                        , str("void")
{
	// string array
	ut::Array<ut::String> arr0;
	ut::Array<ut::String> arr1;
	arr0.Add("FAIL0");
	arr0.Add("FAIL1");
	arr1.Add("FAIL2");
	arr1.Add("FAIL3");
	strarrarr.Add(arr0);
	strarrarr.Add(arr1);

	// initialize dynamic object
	dyn_type_ptr = new PolymorphicA(-1, 1);

	// initialize reflective parameter
	reflective_param = new ReflectiveA(-2, 2);
}

void SerializationTest::Serialize(ut::MetaStream& stream)
{
	stream << ival;
	stream << uval;
	stream << str;
	stream << arr;
	stream << a;
	stream << strarr;
	stream << dyn_type_ptr;
	stream << strarrarr;
	stream << u16ptrarr;

	// reflective dynamic parameter can be
	// changed to the alternate one here
	if (alternate)
	{
		stream << reflective_param_alt;
	}
	else
	{
		stream << reflective_param;
	}

	stream << fval;
}

//----------------------------------------------------------------------------//
TestBase::TestBase() : fval(0.0f)
{ }

const ut::DynamicType& TestBase::Identify() const
{
	return ut::Identify(this);
}

void TestBase::Serialize(ut::MetaStream& stream)
{
	stream << fval;
}

//----------------------------------------------------------------------------//
ReflectiveBase::ReflectiveBase()
{ }

const ut::DynamicType& ReflectiveBase::Identify() const
{
	return ut::Identify(this);
}

void ReflectiveBase::Register(ut::MetaRegistry& registry)
{
	registry.Add(base_str, "base_name");
}

//----------------------------------------------------------------------------//
ReflectiveBaseAlt::ReflectiveBaseAlt()
{ }

const ut::DynamicType& ReflectiveBaseAlt::Identify() const
{
	return ut::Identify(this);
}

void ReflectiveBaseAlt::Register(ut::MetaRegistry& registry)
{
	registry.Add(base_str, "base_name");
}

//----------------------------------------------------------------------------//
PolymorphicA::PolymorphicA(ut::int32 in_ival, ut::uint32 in_uval) : ival(in_ival), uval(in_uval)
{ }

const ut::DynamicType& PolymorphicA::Identify() const
{
	return ut::Identify(this);
}

void PolymorphicA::Serialize(ut::MetaStream& stream)
{
	TestBase::Serialize(stream);
	stream << ival << uval;
}

//----------------------------------------------------------------------------//
PolymorphicB::PolymorphicB(const char* in_str, ut::uint16 in_uval) : str(in_str), uval(in_uval)
{ }

const ut::DynamicType& PolymorphicB::Identify() const
{
	return ut::Identify(this);
}

void PolymorphicB::Serialize(ut::MetaStream& stream)
{
	TestBase::Serialize(stream);
	stream << str << uval;
}

//----------------------------------------------------------------------------//
ReflectiveA::ReflectiveA(ut::int32 in_ival, ut::uint32 in_uval) : ival(in_ival), uval(in_uval)
{ }

const ut::DynamicType& ReflectiveA::Identify() const
{
	return ut::Identify(this);
}

void ReflectiveA::Register(ut::MetaRegistry& registry)
{
	ReflectiveBase::Register(registry);
	registry.Add(ival, "signed_int_parameter");
	registry.Add(uval, "unsigned_int_parameter");
}

//----------------------------------------------------------------------------//
ReflectiveB::ReflectiveB(const char* in_str,
                         ut::uint16 in_uval,
                         ut::int32 in_ival) : alt_str("alternative")
                                            , bstr(in_str)
                                            , uval(in_uval)
                                            , i_ptr(new ut::int32(in_ival))
{ }

const ut::DynamicType& ReflectiveB::Identify() const
{
	return ut::Identify(this);
}

void ReflectiveB::Register(ut::MetaRegistry& registry)
{
	ReflectiveBase::Register(registry);

	// non-existent parameter
	registry.Add(alt_str, "must_be_missing");

	// straight order
	registry.Add(bstr, "b_string_parameter");
	registry.Add(uval, "unsigned_int_parameter");
	registry.Add(i_ptr, "int_ptr_parameter");
	registry.Add(b_ptr_arr, "byte_ptr_array_parameter");
}

//----------------------------------------------------------------------------//

ReflectiveAltB::ReflectiveAltB(const char* in_str,
                               ut::uint16 in_uval,
                               ut::int32 in_ival) : bstr(in_str)
                                                  , uval(in_uval)
                                                  , i_ptr(new ut::int32(in_ival))
{ }

const ut::DynamicType& ReflectiveAltB::Identify() const
{
	return ut::Identify(this);
}

void ReflectiveAltB::Register(ut::MetaRegistry& registry)
{
	ReflectiveBaseAlt::Register(registry);

	// non-existent parameter
	registry.Add(ival, "fictive_int_parameter");

	// change order
	registry.Add(uval, "unsigned_int_parameter");
	registry.Add(bstr, "b_string_parameter");
	registry.Add(b_ptr_arr, "byte_ptr_array_parameter");
	registry.Add(i_ptr, "int_ptr_parameter");
}

//----------------------------------------------------------------------------//
// Changes serialized object to validate it further
// (if loading will fail - parameters would have default values against changed ones)
void ChangeSerializedObject(SerializationTest& object)
{
	object.ival = -0x01234567;
	object.uval = 0x0123456789ABCDEF;
	object.fval = 123.321f;
	object.str = "test_string";

	object.a.u16val = 234;
	object.a.str = "sub_class";
	object.a.iarr.Add(1);
	object.a.iarr.Add(2);
	object.a.iarr.Add(3);

	for (size_t i = 0; i < 3; i++)
	{
		object.strarr.Add("strarr");

		object.u16ptrarr.Add(new ut::uint16(static_cast<ut::uint16>(i) * 2));

		object.arr.Add(SerializationSubClass());
		object.arr[i].u16val = 42;
		object.arr[i].str = "subcarr";
	}

	// array of string arrays
	object.strarrarr.Empty();
	ut::Array<ut::String> arr0;
	arr0.Add("00");
	arr0.Add("01");
	ut::Array<ut::String> arr1;
	arr1.Add("10");
	arr1.Add("11");
	object.strarrarr.Add(arr0);
	object.strarrarr.Add(arr1);

	// change dynamic object
	object.dyn_type_ptr = new PolymorphicB("test_b", 500);
	object.dyn_type_ptr->fval = 1001.504f;

	// change reflective parameter
	object.reflective_param = new ReflectiveB("reflective_b_str", 42, 10);
	object.reflective_param->base_str = "changed";
	ReflectiveB* b_ptr = static_cast<ReflectiveB*>(object.reflective_param.Get());
	b_ptr->b_ptr_arr.Empty();
	b_ptr->b_ptr_arr.Add(new ut::byte(128));
	b_ptr->b_ptr_arr.Add(new ut::byte(64));
	b_ptr->b_ptr_arr.Add(new ut::byte(255));

}

// Checks if serialized object was loaded with the correct values,
// note that ChangeSerializedObject() must be called before saving an object
bool CheckSerializedObject(const SerializationTest& object)
{
	if (object.ival != -0x01234567) return false;
	if (object.uval != 0x0123456789ABCDEF) return false;
	if (object.fval > 123.321f + 0.0001f || object.fval < 123.321f - 0.0001f) return false;
	if (object.str != "test_string") return false;

	if (object.a.u16val != 234) return false;
	if (object.a.str != "sub_class") return false;

	if (object.a.iarr.GetNum() != 3) return false;
	if (object.a.iarr[0] != 1) return false;
	if (object.a.iarr[1] != 2) return false;
	if (object.a.iarr[2] != 3) return false;

	if (object.strarr.GetNum() != 3) return false;
	if (object.strarr[0] != "strarr") return false;
	if (object.strarr[1] != "strarr") return false;
	if (object.strarr[2] != "strarr") return false;
	
	if (object.u16ptrarr.GetNum() != 3) return false;
	if (object.u16ptrarr[0].GetRef() != 0) return false;
	if (object.u16ptrarr[1].GetRef() != 2) return false;
	if (object.u16ptrarr[2].GetRef() != 4) return false;
	
	if (object.arr.GetNum() != 3) return false;
	if (object.arr[0].u16val != 42) return false;
	if (object.arr[1].u16val != 42) return false;
	if (object.arr[2].u16val != 42) return false;
	if (object.arr[0].str != "subcarr") return false;
	if (object.arr[1].str != "subcarr") return false;
	if (object.arr[2].str != "subcarr") return false;
	
	if (object.strarrarr.GetNum() != 2) return false;
	if (object.strarrarr[0].GetNum() != 2) return false;
	if (object.strarrarr[1].GetNum() != 2) return false;
	if (object.strarrarr[0][0] != "00") return false;
	if (object.strarrarr[0][1] != "01") return false;
	if (object.strarrarr[1][0] != "10") return false;
	if (object.strarrarr[1][1] != "11") return false;
	
	// check dynamic type
	if (object.dyn_type_ptr.Get() == nullptr) return false;
	const ut::DynamicType& dyn_type = object.dyn_type_ptr->Identify();
	const ut::String dynamic_type_name(dyn_type.GetName());
	if (dynamic_type_name != "test_derived_B") return false;
	PolymorphicB* dyn_ptr = (PolymorphicB*)object.dyn_type_ptr.Get();
	if (dyn_ptr->str != "test_b") return false;
	if (dyn_ptr->uval != 500) return false;
	if (dyn_ptr->fval < 1001.504f - 0.0001f || dyn_ptr->fval > 1001.504f + 0.0001f) return false;

	// check reflective parameter
	if (object.reflective_param_alt.Get() == nullptr) return false;
	const ut::DynamicType& refl_type = object.reflective_param_alt->Identify();
	const ut::String refl_type_name(refl_type.GetName());
	if (refl_type_name != "reflective_B") return false;
	ReflectiveAltB* refl_ptr = (ReflectiveAltB*)object.reflective_param_alt.Get();
	if (refl_ptr->base_str != "changed") return false;
	if (refl_ptr->uval != 42) return false;
	if (refl_ptr->bstr != "reflective_b_str") return false;
	if (*(refl_ptr->i_ptr.Get()) != 10) return false;
	if (refl_ptr->b_ptr_arr.GetNum() != 3) return false;
	if (*(refl_ptr->b_ptr_arr[0].Get()) != 128) return false;
	if (*(refl_ptr->b_ptr_arr[1].Get()) != 64) return false;
	if (*(refl_ptr->b_ptr_arr[2].Get()) != 255) return false;

	return true;
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
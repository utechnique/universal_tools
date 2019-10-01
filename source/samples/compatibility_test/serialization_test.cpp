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
	tasks.Add(new SerializationVariantsTask);
}

//----------------------------------------------------------------------------//
SerializationVariantsTask::SerializationVariantsTask() : TestTask("Serialization variants")
{
	ut::meta::Info serialization_info = ut::meta::Info::CreateComplete();

	// normal/full
	info_variants.Add(PairType("full", serialization_info));

	// big endian
	serialization_info.SetEndianness(ut::endian::big);
	info_variants.Add(PairType("big endian", serialization_info));

	// no type information
	serialization_info.EnableTypeInformation(false);
	info_variants.Add(PairType("no type information", serialization_info));

	// no linkage information
	serialization_info.EnableLinkageInformation(false);
	info_variants.Add(PairType("no linkage information", serialization_info));

	// no name information
	serialization_info.EnableBinaryNames(false);
	info_variants.Add(PairType("no name information", serialization_info));

	// no value encapsulation
	serialization_info.EnableValueEncapsulation(false);
	info_variants.Add(PairType("no encapsulation", serialization_info));
}

void SerializationVariantsTask::Execute()
{
	report += ut::String("Testing serialization with different header information.") + ut::CRet();
	for (size_t i = 0; i < info_variants.GetNum(); i++)
	{
		report += ut::String("Variant: \"") + info_variants[i].first + "\":" + ut::CRet();
		bool result = TestVariant(info_variants[i].second, info_variants[i].first);
		report += result ? "Success" : "Failed";

		if (i != info_variants.GetNum() - 1)
		{
			report += ut::CRet() + ut::CRet();
		}
	}
}

void SerializationVariantsTask::AddReportEntry(const ut::String& entry)
{
	report += entry + ut::CRet();
}

bool SerializationVariantsTask::TestVariant(const ut::meta::Info& in_info,
                                            const ut::String& name)
{
	// make a copy of provided info structure to override slot for the log
	// signal, so that we could intercept serialization event's description.
	ut::meta::Info info(in_info);
	ut::MemberInvoker<void (SerializationVariantsTask::*)(const ut::String&)>
		log_entry_slot(&SerializationVariantsTask::AddReportEntry, this);
	info.ConnectLogSignalSlot(log_entry_slot);

	// create and change the object
	bool is_mutable = info.HasTypeInformation() && info.HasBinaryNames();
	SerializationTest test_obj(false, in_info.HasLinkageInformation());
	ChangeSerializedObject(test_obj);
	ut::meta::Snapshot snapshot = ut::meta::Snapshot::Capture(test_obj, "test_object", info);

	// you can save serialized object to .xml, .json or .bin file here
	bool save_to_file = false;
	if (save_to_file)
	{
		ut::File xml_file;
		ut::File json_file;
		ut::File binary_file;
		const ut::String xml_filename = name + ".xml";
		const ut::String json_filename = name + ".json";
		const ut::String binary_filename = name + ".bin";

		ut::Optional<ut::Error> open_xml_error = xml_file.Open(xml_filename, ut::file_access_write);
		if (open_xml_error)
		{
			report += "Saving xml file: failed. ";
			report += open_xml_error.Get().GetDesc() + ut::CRet();
			failed_test_counter.Increment();
		}
		else
		{
			ut::XmlDoc xml_doc;
			try
			{
				xml_file << (xml_doc << snapshot);
			}
			catch (const ut::Error& error)
			{
				report += "Saving xml file: failed. ";
				report += error.GetDesc() + ut::CRet();
				failed_test_counter.Increment();
				return false;
			}
		}
		xml_file.Close();

		ut::Optional<ut::Error> open_json_error = json_file.Open(json_filename, ut::file_access_write);
		if (open_json_error)
		{
			report += "Saving json file: failed. ";
			report += open_json_error.Get().GetDesc() + ut::CRet();
			failed_test_counter.Increment();
		}
		else
		{
			ut::JsonDoc json_doc;
			try
			{
				json_file << (json_doc << snapshot);
			}
			catch (const ut::Error& error)
			{
				report += "Saving json file: failed. ";
				report += error.GetDesc() + ut::CRet();
				failed_test_counter.Increment();
				return false;
			}
		}
		json_file.Close();

		ut::Optional<ut::Error> open_binary_error = binary_file.Open(binary_filename, ut::file_access_write);
		if (open_binary_error)
		{
			report += "Saving binary file: failed. ";
			report += open_binary_error.Get().GetDesc() + ut::CRet();
			failed_test_counter.Increment();
			return false;
		}
		else
		{
			ut::Optional<ut::Error> save_error = snapshot.Save(binary_file);
			if (save_error)
			{
				report += "Saving binary file: failed. ";
				report += save_error.Get().GetDesc() + ut::CRet();
				failed_test_counter.Increment();
				return false;
			}
		}
		binary_file.Close();
	}

	// binary serialization
	ut::BinaryStream binary_stream;
	ut::Optional<ut::Error> save_error = snapshot.Save(binary_stream);
	if (save_error)
	{
		report += ut::String("Failed to save binary object.") + ut::CRet();
		report += save_error.Get().GetDesc();
		failed_test_counter.Increment();
		return false;
	}
	binary_stream.MoveCursor(0); // move stream cursor back

	// xml serialization
	ut::BinaryStream xml_stream;
	ut::XmlDoc save_xml;
	try
	{
		xml_stream << (save_xml << snapshot);
	}
	catch (const ut::Error& error)
	{
		report += "Saving xml file: failed. ";
		report += error.GetDesc() + ut::CRet();
		failed_test_counter.Increment();
		return false;
	}
	xml_stream.MoveCursor(0);

	// json serialization
	ut::BinaryStream json_stream;
	ut::JsonDoc save_json;
	try
	{
		json_stream << (save_json << snapshot);
	}
	catch (const ut::Error& error)
	{
		report += "Saving json file: failed. ";
		report += error.GetDesc() + ut::CRet();
		failed_test_counter.Increment();
		return false;
	}
	json_stream.MoveCursor(0);

	// load another object from the stream, it must be
	// the same as the original one
	SerializationTest binary_object(is_mutable, in_info.HasLinkageInformation());
	ut::meta::Snapshot binary_snapshot = ut::meta::Snapshot::Capture(binary_object, "test_object");
	ut::Optional<ut::Error> load_error = binary_snapshot.Load(binary_stream);
	if (load_error)
	{
		report += ut::String("Failed to load binary object:") + ut::CRet();
		report += load_error.Get().GetDesc() + ut::CRet();
		failed_test_counter.Increment();
		return false;
	}

	SerializationTest xml_object(is_mutable, in_info.HasLinkageInformation());
	ut::meta::Snapshot xml_snapshot = ut::meta::Snapshot::Capture(xml_object, "test_object");
	ut::XmlDoc load_xml;
	try
	{
		xml_stream >> load_xml >> xml_snapshot;
	}
	catch (const ut::Error& error)
	{
		report += "Loading xml file: failed. ";
		report += error.GetDesc() + ut::CRet();
		failed_test_counter.Increment();
		return false;
	}

	SerializationTest json_object(is_mutable, in_info.HasLinkageInformation());
	ut::JsonDoc load_json;
	ut::meta::Snapshot json_snapshot = ut::meta::Snapshot::Capture(json_object, "test_object");
	try
	{
		json_stream >> load_json >> json_snapshot;
	}
	catch (const ut::Error& error)
	{
		report += "Loading json file: failed. ";
		report += error.GetDesc() + ut::CRet();
		failed_test_counter.Increment();
		return false;
	}

	// validate binary object immutability after save/load action
	bool check_ok = true;
	if (!CheckSerializedObject(binary_object, is_mutable, in_info.HasLinkageInformation()))
	{
		report += ut::String("FAIL: Objects don't match after binary serialization/deserialization.") + ut::CRet();
		failed_test_counter.Increment();
		check_ok = false;
	}

	// validate xml object immutability after save/load action
	if (!CheckSerializedObject(xml_object, is_mutable, in_info.HasLinkageInformation()))
	{
		report += ut::String("FAIL: Objects don't match after XML serialization/deserialization.") + ut::CRet();
		failed_test_counter.Increment();
		check_ok = false;
	}

	// validate json object immutability after save/load action
	if (!CheckSerializedObject(json_object, is_mutable, in_info.HasLinkageInformation()))
	{
		report += ut::String("JSON: Objects don't match after XML serialization/deserialization. ") + ut::CRet();
		failed_test_counter.Increment();
		check_ok = false;
	}

	// success
	return check_ok;
}

//----------------------------------------------------------------------------//
SerializationSubClass::SerializationSubClass() : u16val(0), str("void")
{ }

void SerializationSubClass::Reflect(ut::meta::Snapshot& snapshot)
{
	snapshot << u16val;
	snapshot << iarr;
	snapshot << str;
}

//----------------------------------------------------------------------------//
SerializationTest::SerializationTest(bool in_alternate,
                                     bool in_can_have_links) : alternate(in_alternate)
                                                             , can_have_links(in_can_have_links)
                                                             , ival(0)
                                                             , ival_ptr(&ival)
                                                             , ival_const_ptr(&ival)
                                                             , void_ptr(nullptr)
                                                             , int16_unique(new ut::int16(1))
                                                             , uval(0)
                                                             , bool_val(false)
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

	// string pointer
	str_ptr = &strarrarr[0][0];
	str_ptr_ptr = &str_ptr;

	// pointer to the value contained in unique pointer
	i16_ptr = int16_unique.Get();

	// initialize dynamic object
	dyn_type_ptr = new PolymorphicA(-1, 1);

	// initialize reflective parameters
	reflective_param = new ReflectiveA(-2, 2);
	reflect_unique_ptr = new ReflectiveA(0, 0);

	// pointers to compicated types
	refl_ptr = reflect_unique_ptr.Get();
	refl_ptr_ptr = &refl_ptr;
	refl_A_ptr = static_cast<ReflectiveA*>(reflect_unique_ptr.Get());
}

void SerializationTest::Reflect(ut::meta::Snapshot& snapshot)
{
	snapshot << ival;
	if (can_have_links)
	{
		snapshot << ival_ptr;
		snapshot << ival_const_ptr;
		snapshot << void_ptr;
		snapshot << str_ptr_ptr;
		snapshot << str_ptr;
		snapshot << i16_ptr;
		snapshot << refl_ptr;
		snapshot << refl_ptr_ptr;
		snapshot << refl_A_ptr;
	}
	snapshot << int16_unique;
	snapshot << uval;
	snapshot << bool_val;
	snapshot << str;
	snapshot << arr;
	snapshot << a;
	snapshot << strarr;
	snapshot << dyn_type_ptr;
	snapshot << strarrarr;
	snapshot << u16ptrarr;
	snapshot << reflect_unique_ptr;

	// reflective dynamic parameter can be
	// changed to the alternate one here
	if (alternate)
	{
		snapshot << reflective_param_alt;
	}
	else
	{
		snapshot << reflective_param;
	}

	snapshot << void_param;
	snapshot << fval;
}

//----------------------------------------------------------------------------//
TestBase::TestBase() : fval(0.0f)
{ }

const ut::DynamicType& TestBase::Identify() const
{
	return ut::Identify(this);
}

void TestBase::Reflect(ut::meta::Snapshot& snapshot)
{
	snapshot << fval;
}

//----------------------------------------------------------------------------//
ReflectiveBase::ReflectiveBase()
{ }

const ut::DynamicType& ReflectiveBase::Identify() const
{
	return ut::Identify(this);
}

void ReflectiveBase::Reflect(ut::meta::Snapshot& snapshot)
{
	snapshot.Add(base_str, "base_name");
}

//----------------------------------------------------------------------------//
ReflectiveBaseAlt::ReflectiveBaseAlt()
{ }

const ut::DynamicType& ReflectiveBaseAlt::Identify() const
{
	return ut::Identify(this);
}

void ReflectiveBaseAlt::Reflect(ut::meta::Snapshot& snapshot)
{
	snapshot.Add(base_str, "base_name");
}

//----------------------------------------------------------------------------//
PolymorphicA::PolymorphicA(ut::int32 in_ival, ut::uint32 in_uval) : ival(in_ival), uval(in_uval)
{ }

const ut::DynamicType& PolymorphicA::Identify() const
{
	return ut::Identify(this);
}

void PolymorphicA::Reflect(ut::meta::Snapshot& snapshot)
{
	TestBase::Reflect(snapshot);
	snapshot << ival << uval;
}

//----------------------------------------------------------------------------//
PolymorphicB::PolymorphicB(const char* in_str, ut::uint16 in_uval) : str(in_str), uval(in_uval)
{ }

const ut::DynamicType& PolymorphicB::Identify() const
{
	return ut::Identify(this);
}

void PolymorphicB::Reflect(ut::meta::Snapshot& snapshot)
{
	TestBase::Reflect(snapshot);
	snapshot << str << uval;
}

//----------------------------------------------------------------------------//
ReflectiveA::ReflectiveA(ut::int32 in_ival, ut::uint32 in_uval) : ival(in_ival), uval(in_uval)
{ }

const ut::DynamicType& ReflectiveA::Identify() const
{
	return ut::Identify(this);
}

void ReflectiveA::Reflect(ut::meta::Snapshot& snapshot)
{
	ReflectiveBase::Reflect(snapshot);
	snapshot.Add(ival, "signed_int_parameter");
	snapshot.Add(uval, "unsigned_int_parameter");
}

//----------------------------------------------------------------------------//
ReflectiveB::ReflectiveB(const char* in_str,
                         ut::uint16 in_uval,
                         ut::int32 in_ival) : alt_str("alternative")
                                            , ui16(13)
                                            , bstr(in_str)
                                            , uval(in_uval)
                                            , i_ptr(new ut::int32(in_ival))
{ }

const ut::DynamicType& ReflectiveB::Identify() const
{
	return ut::Identify(this);
}

void ReflectiveB::Reflect(ut::meta::Snapshot& snapshot)
{
	ReflectiveBase::Reflect(snapshot);

	// non-existent parameter
	snapshot.Add(alt_str, "must_be_missing");

	// this type won't match in alternative version
	snapshot.Add(ui16, "type_mismatch");

	// straight order
	snapshot.Add(bstr, "b_string_parameter");
	snapshot.Add(uval, "unsigned_int_parameter");
	snapshot.Add(i_ptr, "int_ptr_parameter");
	snapshot.Add(b_ptr_arr, "byte_ptr_array_parameter");
}

//----------------------------------------------------------------------------//

ReflectiveAltB::ReflectiveAltB(const char* in_str,
                               ut::uint16 in_uval,
                               ut::int32 in_ival) : bstr(in_str)
                                                  , ui32(13)
                                                  , uval(in_uval)
                                                  , i_ptr(new ut::int32(in_ival))
{ }

const ut::DynamicType& ReflectiveAltB::Identify() const
{
	return ut::Identify(this);
}

void ReflectiveAltB::Reflect(ut::meta::Snapshot& snapshot)
{
	ReflectiveBaseAlt::Reflect(snapshot);

	// non-existent parameter
	snapshot.Add(ival, "fictive_int_parameter");

	// this type won't match in alternative version
	snapshot.Add(ui32, "type_mismatch");

	// change order
	snapshot.Add(uval, "unsigned_int_parameter");
	snapshot.Add(bstr, "b_string_parameter");
	snapshot.Add(b_ptr_arr, "byte_ptr_array_parameter");
	snapshot.Add(i_ptr, "int_ptr_parameter");
}

//----------------------------------------------------------------------------//
// Changes serialized object to validate it further
// (if loading will fail - parameters would have default values against changed ones)
void ChangeSerializedObject(SerializationTest& object)
{
	object.ival = -0x01234567;
	object.uval = 0x0123456789ABCDEF;
	object.bool_val = true;
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

	// string pointer
	object.str_ptr = &object.strarrarr[0][0];

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
bool CheckSerializedObject(const SerializationTest& object, bool alternate, bool linkage)
{
	if (object.ival != -0x01234567) return false;
	if (object.uval != 0x0123456789ABCDEF) return false;
	if (object.bool_val != true) return false;
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
	if (alternate)
	{
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
	}
	else
	{
		if (object.reflective_param.Get() == nullptr) return false;
		const ut::DynamicType& refl_type = object.reflective_param->Identify();
		const ut::String refl_type_name(refl_type.GetName());
		if (refl_type_name != "reflective_B") return false;
	}

	// links
	if (linkage)
	{
		if (object.ival_ptr == nullptr) return false;
		if (object.ival_const_ptr == nullptr) return false;
		if (*object.ival_ptr != object.ival) return false;
		if (*object.ival_const_ptr != object.ival) return false;
		if (object.void_ptr != nullptr) return false;
		if (object.str_ptr != &object.strarrarr[0][0]) return false;
		if (object.str_ptr_ptr != &object.str_ptr) return false;
		if (object.i16_ptr != object.int16_unique.Get()) return false;
		if (*object.i16_ptr != 1) return false;
		if (object.refl_ptr != object.reflect_unique_ptr.Get()) return false;
		if (object.refl_ptr_ptr != &object.refl_ptr) return false;
		if (object.refl_A_ptr != object.reflect_unique_ptr.Get()) return false;
	}

	return true;
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "serialization_test.h"
//----------------------------------------------------------------------------//
// simple polymorphic types
UT_REGISTER_TYPE(TestBase, TestBase, "test_base")
UT_REGISTER_TYPE(TestBase, PolymorphicA, "test_derived_A")
UT_REGISTER_TYPE(TestBase, PolymorphicB, "test_derived_B")
UT_REGISTER_TYPE(PolymorphicC, PolymorphicD, "test_derived_D")
UT_REGISTER_TYPE(PolymorphicD, PolymorphicE, "test_derived_E")
UT_REGISTER_TYPE(PolymorphicB, PolymorphicC, "test_derived_C")

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
	tasks.Add(ut::MakeUnique<SerializationBaseTestTask>());
	tasks.Add(ut::MakeUnique<SerializationVariantsTask>());
	tasks.Add(ut::MakeUnique<PolymorphicClassificationTask>());
}

//----------------------------------------------------------------------------//
SerializationBaseTestTask::SerializationBaseTestTask() : TestTask("Base test")
{ }

void SerializationBaseTestTask::Execute()
{
	ut::DynamicType::Handle handles[5] = {
		ut::GetPolymorphicHandle<PolymorphicA>(),
		ut::GetPolymorphicHandle<PolymorphicB>(),
		ut::GetPolymorphicHandle<PolymorphicC>(),
		ut::GetPolymorphicHandle<PolymorphicD>(),
		ut::GetPolymorphicHandle<PolymorphicE>()
	};

	report += "Handles: ";
	for (size_t i = 0; i < 5; i++)
	{
		for (size_t j = 0; j < 5; j++)
		{
			if (i == j)
			{
				continue;
			}

			if (handles[i] == handles[j])
			{
				report += "Error! handles are not unique: ";
				report += ut::Print(i) + " and " + ut::Print(j);
				failed_test_counter.Increment();
				return;
			}
		}

		ut::uint64 hn = static_cast<ut::uint64>(handles[i]);
		ut::String h_str;
		h_str.Print("0x%llx", hn);
		report += h_str + " ";
	}

	report += "Success ";
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

	// no name information
	serialization_info.EnableBinaryNames(false);
	info_variants.Add(PairType("no name information", serialization_info));

	// no linkage information
	serialization_info.EnableLinkageInformation(false);
	info_variants.Add(PairType("no linkage information", serialization_info));

	// no value encapsulation
	serialization_info.EnableValueEncapsulation(false);
	info_variants.Add(PairType("no encapsulation", serialization_info));

	// minimal
	serialization_info = ut::meta::Info::CreateMinimal();
	info_variants.Add(PairType("minimal", serialization_info));
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
	auto report_function = ut::MemberFunction<SerializationVariantsTask, void(const ut::String&)>
		(this, &SerializationVariantsTask::AddReportEntry);
	info.ConnectLogSignalSlot(ut::Move(report_function));

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
			report += open_xml_error->GetDesc() + ut::CRet();
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
			report += open_json_error->GetDesc() + ut::CRet();
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
			report += open_binary_error->GetDesc() + ut::CRet();
			failed_test_counter.Increment();
			return false;
		}
		else
		{
			ut::Optional<ut::Error> save_error = snapshot.Save(binary_file);
			if (save_error)
			{
				report += "Saving binary file: failed. ";
				report += save_error->GetDesc() + ut::CRet();
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
		report += save_error->GetDesc();
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
		report += load_error->GetDesc() + ut::CRet();
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
		report += ut::String("FAIL: Objects don't match after JSON serialization/deserialization. ") + ut::CRet();
		failed_test_counter.Increment();
		check_ok = false;
	}

	// success
	return check_ok;
}

//----------------------------------------------------------------------------//
PolymorphicClassificationTask::PolymorphicClassificationTask() : TestTask("Classification")
{}

void PolymorphicClassificationTask::Execute()
{
	ut::Array< ut::UniquePtr<TestBase> > base;

	base.Add(ut::MakeUnique<PolymorphicA>());
	base.Add(ut::MakeUnique<PolymorphicA>());
	base.Add(ut::MakeUnique<PolymorphicA>());
	base.Add(ut::MakeUnique<PolymorphicB>());
	base.Add(ut::MakeUnique<PolymorphicC>());
	base.Add(ut::MakeUnique<PolymorphicC>());
	base.Add(ut::MakeUnique<PolymorphicD>());
	base.Add(ut::MakeUnique<PolymorphicE>());


	ut::Array< ut::Ref<PolymorphicA> > derived_a;
	ut::Factory<TestBase>::Select<PolymorphicA>(base, derived_a);

	if (derived_a.GetNum() != 3)
	{
		report += ut::String("FAILED: Invalid number of derived objects.") + ut::CRet();
		failed_test_counter.Increment();
		return;
	}

	ut::meta::Selector<
		TestBase,
		PolymorphicA,
		PolymorphicB,
		PolymorphicC
	> selector;

	selector.Select(base);

	ut::Array< ut::Ref<PolymorphicA> >& a = selector.Get<PolymorphicA>();
	ut::Array< ut::Ref<PolymorphicB> >& b = selector.Get<PolymorphicB>();
	ut::Array< ut::Ref<PolymorphicC> >& c = selector.Get<PolymorphicC>();

	if (a.GetNum() != 3 || b.GetNum() != 1 || c.GetNum() != 2)
	{
		report += ut::String("FAILED: Invalid number of derived objects.") + ut::CRet();
		failed_test_counter.Increment();
		return;
	}

	selector.Reset();

	if (a.GetNum() != 0 || b.GetNum() != 0 || c.GetNum() != 0)
	{
		report += ut::String("FAILED: non-empty array after a Reset() call.") + ut::CRet();
		failed_test_counter.Increment();
		return;
	}

	if (a.GetCapacity() == 0 || b.GetCapacity() == 0 || c.GetCapacity() == 0)
	{
		report += ut::String("FAILED: capacity became 0 after a Reset() call.") + ut::CRet();
		failed_test_counter.Increment();
		return;
	}

	selector.Select(base);

	if (a.GetNum() != 3 || b.GetNum() != 1 || c.GetNum() != 2)
	{
		report += ut::String("FAILED: Invalid number of derived objects.") + ut::CRet();
		failed_test_counter.Increment();
		return;
	}

	report += "Ok.";
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
                                                             , ival2(2)
                                                             , matrix(0.0f)
                                                             , vector(0.0f)
                                                             , color(0)
                                                             , rect(0.0f, 0.0f, 0.0f, 0.0f)
                                                             , quaternion(0.0f, 0.0f, 0.0f, 0.0f)
                                                             , ival_ptr(&ival)
                                                             , ival_const_ptr(&ival)
                                                             , void_ptr(nullptr)
                                                             , ival_ptr2(&iholder.ival2)
                                                             , int16_unique(ut::MakeUnique<ut::int16>(1))
                                                             , int16_unique_void(ut::MakeUnique<ut::int16>(2))
                                                             , uval(0)
                                                             , bool_val(false)
                                                             , fval(0.0f)
                                                             , str("void")
{
	// static array
	for (size_t i = 0; i < 12; i++)
	{
		ival_arr[i] = 0;
	}

	for (size_t i = 0; i < 2; i++)
	{
		for (size_t j = 0; j < 2; j++)
		{
			ival_arr_2d[i][j] = 0;
		}
	}

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
	dyn_type_ptr = ut::MakeUnique<PolymorphicA>(-1, 1);
	dyn_ptr_c = ut::MakeUnique<PolymorphicA>(2, 0);
	dyn_ptr_e = ut::MakeUnique<PolymorphicA>(3, 1);
	dyn_ptr_cc = ut::MakeUnique<PolymorphicE>("e_obj");
	dyn_ptr_cd = ut::MakeUnique<PolymorphicC>("c_obj");
	dyn_ptr_ce = ut::MakeUnique<PolymorphicC>("c_obj");

	// initialize reflective parameters
	reflective_param = ut::MakeUnique<ReflectiveA>(-2, 2);
	reflect_unique_ptr = ut::MakeUnique<ReflectiveA>(0, 0);
	reflect_unique_ptr2 = ut::MakeUnique<ReflectiveA>(0, 2);

	// pointers to compicated types
	refl_ptr = reflect_unique_ptr.Get();
	refl_ptr_ptr = &refl_ptr;
	refl_A_ptr = static_cast<ReflectiveA*>(reflect_unique_ptr.Get());

	// shared pointers
	ival_shared_ptr_0 = ut::MakeShared<ut::int32>(2);
	ival_shared_ptr_1 = ival_shared_ptr_0;
	ival_shared_ptr_2 = ut::MakeShared<ut::int32>(102);
	refl_shared_ptr = ut::MakeShared<ReflectiveA>(0, 3);
	refl_shared_void_ptr = ut::MakeShared<ReflectiveA>(1, 1);
	ival_weak_ptr_0 = ival_shared_ptr_0;
	void_weak_ptr_0 = ival_shared_ptr_0;

	// byte array
	byte_data.Resize(256);
	for (size_t i = 0; i < 256; i++)
	{
		byte_data[i] = static_cast<ut::byte>(256 - i);
	}

	// int array, non-default allocator
	al_int_data.Resize(64);
	for (int i = 0; i < 64; i++)
	{
		al_int_data[i] = 64 - i;
	}

	// binary data
	binary0.Resize(512);
	binary1.Resize(512);
	for (int i = 0; i < 512; i++)
	{
		binary0[i] = i;
		binary1[i] = ut::Vector<3, float>(i, i, i);
	}
	binary_matrix = ut::Matrix<4, 4>(0);

	// vector array
	vec_data.Resize(32);
	for (size_t i = 0; i < 32; i++)
	{
		vec_data[i] = ut::Vector<3, ut::byte>(0xf1, 0xf2, 0xf3);
	}
}

void SerializationTest::Reflect(ut::meta::Snapshot& snapshot)
{
	snapshot << ival;
	snapshot << ival_arr;
	snapshot << ival_arr_2d;
	snapshot << ival2;
	snapshot << matrix;
	snapshot << vector;
	snapshot << color;
	snapshot << rect;
	snapshot << quaternion;
	if (can_have_links)
	{
		snapshot << ival_ptr;
		snapshot << ival_const_ptr;
		snapshot << void_ptr;
		snapshot << iholder;
		snapshot << ival_ptr2;
		snapshot << str_ptr_ptr;
		snapshot << str_ptr;
		snapshot << i16_ptr;
		snapshot << refl_ptr;
		snapshot << refl_ptr_ptr;
		snapshot << refl_A_ptr;
		snapshot << ival_shared_ptr_0;
		snapshot << ival_shared_ptr_1;
		snapshot << ival_shared_ptr_2;
		snapshot << refl_shared_ptr;
		snapshot << refl_shared_void_ptr;
		snapshot << refl_shared_level_ptr;
		snapshot << ival_weak_ptr_0;
		snapshot << deep_weak_ptr_0;
		snapshot << void_weak_ptr_0;
	}
	snapshot << byte_data;
	snapshot << al_int_data;
	snapshot << ut::meta::Binary(binary0, 1);
	snapshot << ut::meta::Binary(binary1, sizeof(float));
	snapshot << ut::meta::Binary(binary_matrix, sizeof(float));
	snapshot << vec_data;
	snapshot << int16_unique;
	snapshot << int16_unique_void;
	snapshot << uval;
	snapshot << bool_val;
	snapshot << str;
	snapshot << arr;
	snapshot << a;
	snapshot << strarr;
	snapshot << dyn_type_ptr;
	snapshot << dyn_ptr_c;
	snapshot << dyn_ptr_e;
	snapshot << dyn_ptr_cc;
	snapshot << dyn_ptr_cd;
	snapshot << dyn_ptr_ce;
	snapshot << strarrarr;
	snapshot << u16ptrarr;
	snapshot << avltree;
	snapshot << al_avltree;
	snapshot << reflect_unique_ptr;
	snapshot << reflect_unique_ptr2;

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
PolymorphicC::PolymorphicC(const char* in_str) : str_c(in_str)
{ }

const ut::DynamicType& PolymorphicC::Identify() const
{
	return ut::Identify(this);
}

void PolymorphicC::Reflect(ut::meta::Snapshot& snapshot)
{
	TestBase::Reflect(snapshot);
	snapshot << str_c;
}

//----------------------------------------------------------------------------//
PolymorphicD::PolymorphicD(ut::int16 in_ival16) : ival16_d(in_ival16)
{ }

const ut::DynamicType& PolymorphicD::Identify() const
{
	return ut::Identify(this);
}

void PolymorphicD::Reflect(ut::meta::Snapshot& snapshot)
{
	TestBase::Reflect(snapshot);
	snapshot << ival16_d;
}

//----------------------------------------------------------------------------//
PolymorphicE::PolymorphicE(const char* in_str) : str_e(in_str)
{ }

const ut::DynamicType& PolymorphicE::Identify() const
{
	return ut::Identify(this);
}

void PolymorphicE::Reflect(ut::meta::Snapshot& snapshot)
{
	TestBase::Reflect(snapshot);
	snapshot << str_e;
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
                                            , i_ptr(ut::MakeUnique<ut::int32>(in_ival))
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
                                                  , i_ptr(ut::MakeUnique<ut::int32>(in_ival))
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
SharedTestLevel2::SharedTestLevel2() : i32_shared(ut::MakeShared<ut::int32>(3))
{ }

void SharedTestLevel2::Reflect(ut::meta::Snapshot& snapshot)
{
	snapshot.Add(i32_shared, "level2_int");
}

//----------------------------------------------------------------------------//
SharedTestLevel1::SharedTestLevel1() : shared_obj(ut::MakeShared<SharedTestLevel2>())
{ }

void SharedTestLevel1::Reflect(ut::meta::Snapshot& snapshot)
{
	snapshot.Add(shared_obj, "level1_obj");
}

//----------------------------------------------------------------------------//
SharedTestLevel0::SharedTestLevel0() : shared_obj(ut::MakeShared<SharedTestLevel1>())
{ }

void SharedTestLevel0::Reflect(ut::meta::Snapshot& snapshot)
{
	snapshot.Add(shared_obj, "level0_obj");
}

//----------------------------------------------------------------------------//
IntHolder::IntHolder() : ival(3), ival2(6)
{ }

void IntHolder::Reflect(ut::meta::Snapshot& snapshot)
{
	snapshot.Add(ival, "ival");
	snapshot.Add(ival2, "ival2");
}

//----------------------------------------------------------------------------//
// Changes serialized object to validate it further
// (if loading will fail - parameters would have default values against changed ones)
void ChangeSerializedObject(SerializationTest& object)
{
	for (size_t i = 0; i < 12; i++)
	{
		object.ival_arr[i] = static_cast<ut::int32>(i);
	}

	for (size_t i = 0; i < 2; i++)
	{
		for (size_t j = 0; j < 2; j++)
		{
			object.ival_arr_2d[i][j] = static_cast<ut::int32>(i*2 + j);
		}
	}

	object.matrix = ut::Matrix<4, 4>(0,  1,  2,  3,
	                                 4,  5,  6,  7,
	                                 8,  9,  10, 11,
	                                 12, 13, 14, 15);
	object.vector = ut::Vector<3>(0, 1, 2);
	object.color = ut::Color<3, ut::byte>(10, 11, 12);
	object.rect = ut::Rect<float>(1.0f, 2.0f, 10.0f, 20.0f);
	object.quaternion = ut::Quaternion<double>(0.0f, 1.0f, 2.0f, 3.0f);

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

		object.u16ptrarr.Add(ut::MakeUnique<ut::uint16>(static_cast<ut::uint16>(i) * 2));

		object.arr.Add(SerializationSubClass());
		object.arr[i].u16val = 42;
		object.arr[i].str = "subcarr";
	}

	// avltree
	object.avltree.Insert(1, "__1");
	object.avltree.Insert(55, "__55");
	object.avltree.Insert(4, "__4");
	object.avltree.Insert(3, "__3");
	object.avltree.Insert(5, "__5");

	// avltree (non-default allocator)
	object.al_avltree.Insert(1, "__1");
	object.al_avltree.Insert(55, "__55");
	object.al_avltree.Insert(4, "__4");
	object.al_avltree.Insert(3, "__3");
	object.al_avltree.Insert(5, "__5");

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

	// change dynamic objects
	object.dyn_type_ptr = ut::MakeUnique<PolymorphicB>("test_b", 500);
	object.dyn_type_ptr->fval = 1001.504f;
	object.dyn_ptr_c = ut::MakeUnique<PolymorphicC>("c_obj");
	object.dyn_ptr_e = ut::MakeUnique<PolymorphicE>("e_obj");
	object.dyn_ptr_cc = ut::MakeUnique<PolymorphicC>("c_obj");
	object.dyn_ptr_cd = ut::MakeUnique<PolymorphicD>(10);
	object.dyn_ptr_ce = ut::MakeUnique<PolymorphicE>("e_obj");

	// change reflective parameter
	object.reflective_param = ut::MakeUnique<ReflectiveB>("reflective_b_str", 42, 10);
	object.reflective_param->base_str = "changed";
	ReflectiveB* b_ptr = static_cast<ReflectiveB*>(object.reflective_param.Get());
	b_ptr->b_ptr_arr.Empty();
	b_ptr->b_ptr_arr.Add(ut::MakeUnique<ut::byte>(128));
	b_ptr->b_ptr_arr.Add(ut::MakeUnique<ut::byte>(64));
	b_ptr->b_ptr_arr.Add(ut::MakeUnique<ut::byte>(255));

	// change pointers
	object.int16_unique_void.Delete();
	object.ival_const_ptr = &object.ival2;
	object.ival_ptr2 = &object.iholder.ival;
	object.refl_ptr = object.reflect_unique_ptr2.Get();

	// change shared pointers
	object.ival_shared_ptr_0 = ut::MakeShared<ut::int32>(4);
	object.ival_shared_ptr_1 = object.ival_shared_ptr_0;
	object.ival_shared_ptr_2 = ut::MakeShared<ut::int32>(101);
	object.refl_shared_ptr = ut::MakeShared<ReflectiveB>("reflective_b_str", 1, 1);
	object.refl_shared_void_ptr.Reset();
	object.refl_shared_level_ptr = ut::MakeShared<SharedTestLevel0>();
	object.refl_shared_level_ptr->shared_obj->shared_obj->i32_shared.GetRef() = 114;

	// weak pointers
	object.ival_weak_ptr_0 = object.ival_shared_ptr_2;
	object.deep_weak_ptr_0 = object.refl_shared_level_ptr->shared_obj->shared_obj->i32_shared;
	object.void_weak_ptr_0.Reset();

	// byte array
	object.byte_data.Resize(256);
	for (size_t i = 0; i < 256; i++)
	{
		object.byte_data[i] = static_cast<ut::byte>(i);
	}

	// int array, non-default allocator
	object.al_int_data.Resize(64);
	for (int i = 0; i < 64; i++)
	{
		object.al_int_data[i] = i;
	}

	// binary data
	object.binary0.Resize(256);
	object.binary1.Resize(256);
	for (int i = 0; i < 256; i++)
	{
		object.binary0[i] = 255 - i;
		object.binary1[i] = ut::Vector<3, float>(255 - i, i, 255 + i);
	}
	object.binary_matrix = ut::Matrix<4, 4>(0,  1,  2,  3,
	                                        4,  5,  6,  7,
	                                        8,  9,  10, 11,
	                                        12, 13, 14, 15);
}

// Checks if serialized object was loaded with the correct values,
// note that ChangeSerializedObject() must be called before saving an object
bool CheckSerializedObject(const SerializationTest& object, bool alternate, bool linkage)
{
	for (size_t i = 0; i < 12; i++)
	{
		if (object.ival_arr[i] != static_cast<ut::int32>(i))
		{
			return false;
		}
	}

	for (size_t i = 0; i < 2; i++)
	{
		for (size_t j = 0; j < 2; j++)
		{
			if (object.ival_arr_2d[i][j] != static_cast<ut::int32>(i * 2 + j))
			{
				return false;
			}
		}
	}

	if (object.matrix != ut::Matrix<4, 4>(0,  1,  2,  3,
	                                      4,  5,  6,  7,
	                                      8,  9,  10, 11,
	                                      12, 13, 14, 15))
	{
		return false;
	}
	if (object.vector != ut::Vector<3>(0, 1, 2))
	{
		return false;
	}
	if (object.color != ut::Color<3, ut::byte>(10, 11, 12))
	{
		return false;
	}
	if (object.rect.offset != ut::Vector<2, float>(1, 2) || object.rect.extent != ut::Vector<2, float>(10, 20))
	{
		return false;
	}
	if (object.quaternion != ut::Quaternion<double>(0.0, 1.0, 2.0, 3.0))
	{
		return false;
	}

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
	
	// avltree
	ut::Optional<const ut::String&> avl_find_result = object.avltree.Find(3);
	if (avl_find_result)
	{
		const ut::String& str = avl_find_result.Get();
		if (str != "__3")
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	// avltree (non-default allocator)
	avl_find_result = object.al_avltree.Find(3);
	if (avl_find_result)
	{
		const ut::String& str = avl_find_result.Get();
		if (str != "__3")
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	// check dynamic type B
	if (object.dyn_type_ptr.Get() == nullptr) return false;
	const ut::DynamicType& dyn_type = object.dyn_type_ptr->Identify();
	const ut::String dynamic_type_name(dyn_type.GetName());
	if (dynamic_type_name != "test_derived_B") return false;
	PolymorphicB* dyn_ptr = (PolymorphicB*)object.dyn_type_ptr.Get();
	if (dyn_ptr->str != "test_b") return false;
	if (dyn_ptr->uval != 500) return false;
	if (dyn_ptr->fval < 1001.504f - 0.0001f || dyn_ptr->fval > 1001.504f + 0.0001f) return false;

	// check dynamic type C
	if (object.dyn_ptr_c.Get() == nullptr) return false;
	const ut::DynamicType& dyn_type_c = object.dyn_ptr_c->Identify();
	const ut::String dynamic_type_name_c(dyn_type_c.GetName());
	if (dynamic_type_name_c != "test_derived_C") return false;
	PolymorphicC* dyn_ptr_c = (PolymorphicC*)object.dyn_ptr_c.Get();
	if (dyn_ptr_c->str_c != "c_obj") return false;
	
	// check dynamic type E
	if (object.dyn_ptr_e.Get() == nullptr) return false;
	const ut::DynamicType& dyn_type_e = object.dyn_ptr_e->Identify();
	const ut::String dynamic_type_name_e(dyn_type_e.GetName());
	if (dynamic_type_name_e != "test_derived_E") return false;
	PolymorphicE* dyn_ptr_e = (PolymorphicE*)object.dyn_ptr_e.Get();
	if (dyn_ptr_e->str_e != "e_obj") return false;

	// check dynamic type CC
	if (object.dyn_ptr_cc.Get() == nullptr) return false;
	const ut::DynamicType& dyn_type_cc = object.dyn_ptr_cc->Identify();
	const ut::String dynamic_type_name_cc(dyn_type_cc.GetName());
	if (dynamic_type_name_cc != "test_derived_C") return false;
	PolymorphicC* dyn_ptr_cc = (PolymorphicC*)object.dyn_ptr_cc.Get();
	if (dyn_ptr_cc->str_c != "c_obj") return false;

	// check dynamic type CD
	if (object.dyn_ptr_cd.Get() == nullptr) return false;
	const ut::DynamicType& dyn_type_cd = object.dyn_ptr_cd->Identify();
	const ut::String dynamic_type_name_cd(dyn_type_cd.GetName());
	if (dynamic_type_name_cd != "test_derived_D") return false;
	PolymorphicD* dyn_ptr_cd = (PolymorphicD*)object.dyn_ptr_cd.Get();
	if (dyn_ptr_cd->ival16_d != 10) return false;

	// check dynamic type CE
	if (object.dyn_ptr_ce.Get() == nullptr) return false;
	const ut::DynamicType& dyn_type_ce = object.dyn_ptr_ce->Identify();
	const ut::String dynamic_type_name_ce(dyn_type_ce.GetName());
	if (dynamic_type_name_ce != "test_derived_E") return false;
	PolymorphicE* dyn_ptr_ce = (PolymorphicE*)object.dyn_ptr_ce.Get();
	if (dyn_ptr_ce->str_e != "e_obj") return false;

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
		if (*object.ival_const_ptr != object.ival2) return false;
		if (object.void_ptr != nullptr) return false;
		if (*object.ival_ptr2 != object.iholder.ival) return false;
		if (object.str_ptr != &object.strarrarr[0][0]) return false;
		if (object.str_ptr_ptr != &object.str_ptr) return false;
		if (object.i16_ptr != object.int16_unique.Get()) return false;
		if (*object.i16_ptr != 1) return false;
		if (object.refl_ptr != object.reflect_unique_ptr2.Get()) return false;
		if (object.refl_ptr_ptr != &object.refl_ptr) return false;
		if (object.refl_A_ptr != object.reflect_unique_ptr.Get()) return false;
		if (object.int16_unique_void.Get() != nullptr) return false;

		// shared
		if (object.ival_shared_ptr_0.GetRef() != 4) return false;
		if (object.ival_shared_ptr_1.GetRef() != 4) return false;
		if (object.ival_shared_ptr_2.GetRef() != 101) return false;
		if (object.refl_shared_void_ptr.Get() != nullptr) return false;
		if (object.refl_shared_ptr.Get() == nullptr) return false;
		const ut::DynamicType& refl_type = object.refl_shared_ptr->Identify();
		const ut::String refl_type_name(refl_type.GetName());
		if (refl_type_name != "reflective_B") return false;
		if (object.refl_shared_level_ptr)
		{
			if (object.refl_shared_level_ptr->shared_obj)
			{
				if (object.refl_shared_level_ptr->shared_obj->shared_obj)
				{
					if (object.refl_shared_level_ptr->shared_obj->shared_obj->i32_shared)
					{
						if (object.refl_shared_level_ptr->shared_obj->shared_obj->i32_shared.GetRef() != 114) return false;
					}
					else
					{
						return false;
					}
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}


		// weak
		if (object.ival_weak_ptr_0.IsValid())
		{
			ut::SharedPtr<ut::int32> pinned = object.ival_weak_ptr_0.Pin();
			if (pinned.Get() != object.ival_shared_ptr_2.Get())
			{
				return false;
			}
		}
		else
		{
			return false;
		}

		if (object.deep_weak_ptr_0.IsValid())
		{
			ut::SharedPtr<ut::int32> pinned = object.deep_weak_ptr_0.Pin();
			if (pinned.GetRef() != 114)
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	
		if (object.void_weak_ptr_0.IsValid()) return false;
	}

	// byte array
	if (object.byte_data.GetSize() != 256) return false;
	for (size_t i = 0; i < 256; i++)
	{
		if (object.byte_data[i] != i)
		{
			return false;
		}
	}

	// int array, non-default allocator
	if (object.al_int_data.GetNum() != 64) return false;
	for (int i = 0; i < 64; i++)
	{
		if (object.al_int_data[i] != i)
		{
			return false;
		}
	}

	// binary data
	if (object.binary0.GetNum() != 256) return false;
	if (object.binary1.GetNum() != 256) return false;
	for (int i = 0; i < 256; i++)
	{
		if (object.binary0[i] != 255 - i)
		{
			return false;
		}

		if (object.binary1[i] != ut::Vector<3, float>(255 - i, i, 255 + i))
		{
			return false;
		}
	}
	if (object.binary_matrix != ut::Matrix<4, 4>(0,  1,  2,  3,
	                                             4,  5,  6,  7,
	                                             8,  9,  10, 11,
	                                             12, 13, 14, 15))
	{
		return false;
	}

	return true;
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
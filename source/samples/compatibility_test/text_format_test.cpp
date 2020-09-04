//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "text_format_test.h"
//----------------------------------------------------------------------------//
// Unit
TextFormatUnit::TextFormatUnit() : TestUnit("TEXT FORMAT")
{
	tasks.Add(ut::MakeUnique<XmlTask>());
	tasks.Add(ut::MakeUnique<JsonTask>());
}

//----------------------------------------------------------------------------//
// Host name
XmlTask::XmlTask() : TestTask("XML") {}

void XmlTask::Execute()
{
	ut::XmlDoc doc;
	ut::Optional<ut::Error> parse_error = doc.Parse(g_xml_file_contents);
	if (parse_error)
	{
		report += "failed to parse file test xml document: \n";
		report += parse_error->GetDesc();
		failed_test_counter.Increment();
		return; // exit
	}
	else
	{
		report += "xml document was parsed successfully. ";
	}

	ut::BinaryStream binary_stream;
	ut::Optional<ut::Error> save_error = doc.Write(binary_stream);
	if (save_error)
	{
		report += "Failed to save test xml document: ";
		report += save_error->GetDesc();
		failed_test_counter.Increment();
	}
	else
	{
		report += "Test xml file saved successfully.";

		// you can save xml doc to file for test purposes
		const bool save_to_file = false;
		if (save_to_file)
		{
			ut::File file;
			if (!file.Open("test.xml", ut::file_access_write))
			{
				file.Write(binary_stream.GetData().Get(), 1, binary_stream.GetSize().Get());
				file.Close();
			}
		}
	}
}

//----------------------------------------------------------------------------//
// Host name
JsonTask::JsonTask() : TestTask("JSon") {}

void JsonTask::Execute()
{
	ut::JsonDoc doc;
	ut::Optional<ut::Error> parse_error = doc.Parse(g_json_file_contents);
	if (parse_error)
	{
		report += "failed to parse file test json document: \n";
		report += parse_error->GetDesc();
		failed_test_counter.Increment();
		return; // exit
	}
	else
	{
		report += "json document was parsed successfully. ";
	}

	ut::BinaryStream binary_stream;
	ut::Optional<ut::Error> save_error = doc.Write(binary_stream);
	if (save_error)
	{
		report += "Failed to save test json document: ";
		report += save_error->GetDesc();
		failed_test_counter.Increment();
	}
	else
	{
		report += "Test json file saved successfully.";

		// you can save json doc to file for test purposes
		const bool save_to_file = false;
		if (save_to_file)
		{
			ut::File file;
			if (!file.Open("test.json", ut::file_access_write))
			{
				file.Write(binary_stream.GetData().Get(), 1, binary_stream.GetSize().Get());
				file.Close();
			}
		}
	}

	// convert to xml
	ut::XmlDoc xml_doc;
	xml_doc.nodes = doc.nodes;
	const bool convert_to_xml = false;
	if (convert_to_xml)
	{
		report += "Converting json to xml: ";
		ut::File file;
		if (!file.Open("conversion_test.xml", ut::file_access_write))
		{
			ut::Optional<ut::Error> save_error = xml_doc.Write(file);
			if (save_error)
			{
				report += "Failed to convert test json document: ";
				report += save_error->GetDesc();
				failed_test_counter.Increment();
			}
			else
			{
				report += "Test json->xml file saved successfully.";
			}
			file.Close();
		}
	}
}

//----------------------------------------------------------------------------//

const char* g_xml_file_contents =
	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
	"<!DOCTYPE breakfast_menu>"
	"<breakfast_menu>"
	"	<?xml-stylesheet href=\"mystyle.xslt\" type=\"text / xsl\"?>"
	"	<!--Commentary node-->"
	"	<![CDATA[\"cdata test\"]]>"
	"	<food type=\"restaurant\" salt=\"yes\">"
	"		<name>Belgian Waffles</name>"
	"		<price>$5.95</price>"
	"		<description>"
	"			Two of our famous Belgian Waffles with plenty of real maple syrup"
	"		</description>"
	"		<calories>650</calories>"
	"	</food>"
	"	<food>"
	"		<name>Strawberry Belgian Waffles</name>"
	"		<price>$7.95</price>"
	"		<description>"
	"			Light Belgian waffles covered with strawberries and whipped cream"
	"		</description>"
	"		<calories>900</calories>"
	"	</food>"
	"	<food>"
	"		<name>Berry-Berry Belgian Waffles</name>"
	"		<price>$8.95</price>"
	"		<description>"
	"			Light Belgian waffles covered with an assortment of fresh berries and whipped cream"
	"		</description>"
	"		<calories>900</calories>"
	"	</food>"
	"	<food>"
	"		<name>French Toast</name>"
	"		<price>$4.50</price>"
	"		<description>"
	"			Thick slices made from our homemade sourdough bread"
	"		</description>"
	"		<calories>600</calories>"
	"	</food>"
	"	<food>"
	"		<name>Homestyle Breakfast</name>"
	"		<price>$6.95</price>"
	"		<description>"
	"			Two eggs, bacon or sausage, toast, and our ever-popular hash browns"
	"		</description>"
	"		<calories>950</calories>"
	"	</food>"
	"</breakfast_menu>";

//----------------------------------------------------------------------------//

const char* g_json_file_contents =
"{\"widget\": {"
"    \"debug\": \"on\","
"    \"window\": {"
"        \"title\": \"Sample Konfabulator Widget\","
"        \"name\": \"main_window\","
"        \"width\": 500,"
"        \"height\": 500"
"    },"
"    \"image\": {"
"        \"src\": \"Images/Sun.png\","
"        \"name\": \"sun1\","
"        \"hOffset\": 250,"
"        \"vOffset\": 250,"
"        \"alignment\": \"center\""
"    },"
"    \"text\": {"
"        \"data\": \"Click Here\","
"        \"size\": 36,"
"        \"style\": \"bold\","
"        \"name\": \"text1\","
"        \"hOffset\": 250,"
"        \"vOffset\": 100,"
"        \"alignment\": \"center\","
"        \"onMouseUp\": \"sun1.opacity = (sun1.opacity / 100) * 90;\""
"    },"
"	\"cfg\" : {"
"		\"style\" : \"bold\","
"		\"animation\" : true,"
"		\"hints\" : false,"
"		\"log\" : null,"
"		\"tabulation\" : 4"
"	},"
"	\"menuitem\": ["
"		{\"value\": \"New\", \"onclick\": \"CreateNewDoc()\"},"
"		{\"value\": \"Open\", \"onclick\": \"OpenDoc()\"},"
"		{\"value\": \"Close\", \"onclick\": \"CloseDoc()\"}"
"	]"
"}"
"}";

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
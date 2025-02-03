//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "containers_test.h"
#include <unordered_map> // to compare with hasmap
//----------------------------------------------------------------------------//
ContainersTestUnit::ContainersTestUnit() : TestUnit("CONTAINERS")
{
	tasks.Add(ut::MakeUnique<ArrayOpsTask>());
	tasks.Add(ut::MakeUnique<TreeTask>());
	tasks.Add(ut::MakeUnique<AVLTreeTask>());
	tasks.Add(ut::MakeUnique<HashmapTask>());
	tasks.Add(ut::MakeUnique<SharedPtrTask>());
	tasks.Add(ut::MakeUnique<TupleTask>());
	tasks.Add(ut::MakeUnique<OptionalTask>());
	tasks.Add(ut::MakeUnique<ResultTask>());
	tasks.Add(ut::MakeUnique<PairTask>());
	tasks.Add(ut::MakeUnique<SmartPtrTask>());
}

//----------------------------------------------------------------------------//

ut::Array<int> GenerateMapArray(size_t size)
{
	unsigned int seed = 5323;
	ut::Array<int> out;

	for (size_t i = 0; i < size; i++)
	{
		seed = (8253729 * seed + 2396403);
		out.Add(seed % 32767);
	}

	for (size_t i = out.Count(); i-- > 0; )
	{
		for (size_t j = i; j-- > 0;)
		{
			if (out[i] == out[j])
			{
				out.Remove(j);
				i--;
			}
		}
	}

	return out;
}

//----------------------------------------------------------------------------//
ArrayOpsTask::ArrayOpsTask() : TestTask("Array ops")
{ }

void ArrayOpsTask::Execute()
{
	// array test, following calls can cause
	// throwing an error if something went wrong:
	ut::Array<int> arr;
	arr.Add(1);
	arr.Add(13);
	arr.Add(3);
	arr.Add(63);
	arr.Add(101);
	arr.Add(22);
	arr.Add(31);
	int a = arr[0] + arr[1];
	arr.Add(a);
	arr.PushForward(0);
	arr.Insert(arr.Begin() + 2, 2);
	arr.Insert(4, 5);

	// test move constructor
	ut::Array<int> arr2;
	arr2 = ut::Move(arr);

	// test last element reference
	int last = arr2.GetLast();
	report.Print("testing array operations, final value must be 14: %i", last);
	if (last != 14)
	{
		failed_test_counter.Increment();
	}

	// test removing an element
	arr2.Remove(arr2.Begin() + 3);

	// test array concatenation
	report += ". Testing array concatenation:";
	ut::Array<int> iarr0;
	int ii = 0;
	iarr0.Add(0);
	iarr0.Add(1);
	ut::Array<int> iarr1;
	iarr0.Add(2);
	iarr0.Add(3);
	iarr0.Add(4);
	ut::Array<int> iarr2 = iarr0 + iarr1;
	iarr0 += iarr1;
	if (iarr2.Count() != 5 || iarr0.Count() != 5)
	{
		report += " FAIL. ";
		failed_test_counter.Increment();
	}
	else if (iarr0[0] != 0 || iarr0[1] != 1 || iarr0[2] != 2 || iarr0[3] != 3 || iarr0[4] != 4)
	{
		report += " FAIL. ";
		failed_test_counter.Increment();
	}
	else if (iarr2[0] != 0 || iarr2[1] != 1 || iarr2[2] != 2 || iarr2[3] != 3 || iarr2[4] != 4)
	{
		report += " FAIL. ";
		failed_test_counter.Increment();
	}
	else
	{
		report += " success. ";
	}

	// UniquePtr concatenation
	ut::Array< ut::UniquePtr<int> > uniqarr0;
	ut::Array< ut::UniquePtr<int> > uniqarr1;
	uniqarr0 += ut::Move(uniqarr1);

	// test reverse order iterator
	report += ". Testing array reverse iterator:";
	ut::Array<int>::Iterator riterator;
	for (riterator = arr2.Begin(ut::iterator::Position::last);
	     riterator > arr2.End(ut::iterator::Position::first);
	     --riterator)
	{
		int val = *riterator;
		int& val1 = *riterator;
		ut::String str;
		str.Print("%i ", val);
		report += str;
	}

	// test normal iterator
	report += ". Testing array forward iterator:";
	ut::Array<int>::Iterator iterator;
	for (iterator = arr2.Begin(ut::iterator::Position::first);
	     iterator < arr2.End(ut::iterator::Position::last);
	     ++iterator)
	{
		int val = *iterator;
		int& val1 = *iterator;
		ut::String str;
		str.Print("%i ", val);
		report += str;
	}

	// test zero sized iterator
	ut::Array<int> zarray;
	ut::Array<int>::Iterator ziterator;
	for (ziterator = zarray.Begin(ut::iterator::Position::first);
	     ziterator < zarray.End(ut::iterator::Position::last);
	     ++ziterator)
	{
		int val = *ziterator;
		int& val1 = *ziterator;
		ut::String str;
		str.Print("%i ", val);
		report += str;
	}

	// array with another allocator
	report += ". Testing array with non-default allocator:";
	ut::Array<int, TestAllocator<int> > al_int_arr_0;
	ut::Array<int, TestAllocator<int> > al_int_arr_1;
	ut::Array<int, TestAllocator<int> > al_int_arr_2;
	al_int_arr_0.Resize(3);
	al_int_arr_0[0] = 0;
	al_int_arr_0[1] = 1;
	al_int_arr_0[2] = 2;
	al_int_arr_0.Add(3);

	al_int_arr_1 = al_int_arr_0;
	al_int_arr_2 = ut::Move(al_int_arr_1);
	if (al_int_arr_2.Count() == 4)
	{
		if (al_int_arr_2[0] == 0 &&
			al_int_arr_2[1] == 1 &&
			al_int_arr_2[2] == 2 &&
			al_int_arr_2[3] == 3)
		{
			report += " success.";
		}
		else
		{
			report += " FAIL. ";
			failed_test_counter.Increment();
		}
	}
	else
	{
		report += " FAIL. ";
		failed_test_counter.Increment();
	}

	// find element
	report += ". Find array element:";
	ut::Array<int> icarr;
	icarr.Add(5);
	icarr.Add(4);
	icarr.Add(3);
	icarr.Add(2);
	icarr.Add(1);
	icarr.Add(0);
	auto find_result = ut::Find(icarr.Begin(), icarr.End(), 3);
	if (find_result)
	{
		if (*find_result.Get() == 3)
		{
			report += " success.";
		}
		else
		{
			report += " FAIL. ";
			failed_test_counter.Increment();
		}
	}
	else
	{
		report += " FAIL. ";
		failed_test_counter.Increment();
	}

	// find element (predicate)
	report += ". Find array element (predicate):";
	struct FindTest
	{
        FindTest(int v) : value(v) {}
		int value = 0;
	};
	ut::Array<FindTest> tarr;
	tarr.Add(FindTest(0));
	tarr.Add(FindTest(1));
	tarr.Add(FindTest(2));
	tarr.Add(FindTest(3));
	tarr.Add(FindTest(4));
	tarr.Add(FindTest(5));
	auto find_result_if = ut::FindIf(tarr.Begin(), tarr.End(), [](const FindTest& t) { return t.value == 4; });
	if (find_result_if)
	{
		if (find_result_if.Get()->value == 4)
		{
			report += " success.";
		}
		else
		{
			report += " FAIL. ";
			failed_test_counter.Increment();
		}
	}
	else
	{
		report += " FAIL. ";
		failed_test_counter.Increment();
	}

	// find element (predicate not)
	report += ". Find array element (predicate not):";
	find_result_if = ut::FindIfNot(tarr.Begin(), tarr.End(), [](const FindTest& t) { return t.value == 0; });
	if (find_result_if)
	{
		if (find_result_if.Get()->value == 1)
		{
			report += " success.";
		}
		else
		{
			report += " FAIL. ";
			failed_test_counter.Increment();
		}
	}
	else
	{
		report += " FAIL. ";
		failed_test_counter.Increment();
	}

	// range based for loop
	report += ". Range based for loop:";
	int sum = 0;
	const ut::Array<FindTest>& ctarrref = tarr;
	for (const FindTest& e : ctarrref)
	{
		sum += e.value;
	}
	for (FindTest& e : tarr)
	{
		sum += e.value;
		e.value++;
	}
	if (sum == 30)
	{
		report += " success.";
	}
	else
	{
		report += " FAIL. ";
		failed_test_counter.Increment();
	}
}

//----------------------------------------------------------------------------//
TreeTask::TreeTask() : TestTask("Tree")
{ }

void TreeTask::Execute()
{
	// tree test, following calls can cause
	// throwing an error if something went wrong:
	ut::Tree<ut::String> tree;
	tree.data = "0";
	report += "adding nodes: ";
	tree.Add("0_0");
	tree.Add("0_1");
	tree.Add("0_2");

	// test how many children
	if (tree.CountChildren() == 3)
	{
		bool success = true;
		for (size_t i = 0; i < 3; i++)
		{
			// trying to add inner nodes
			for (size_t j = 0; j < 3; j++)
			{
				ut::String val = ut::Print<ut::uint32>((ut::uint32)j);
				tree[i].Add(tree[i].data + "_" + val);
			}

			// validate inner nodes
			if (tree[i].CountChildren() != 3)
			{
				success = false;
				report += "Failed to add inner nodes. ";
				failed_test_counter.Increment();
			}
		}

		if (success)
		{
			report += "Success. ";
		}
	}
	else
	{
		report += "Failed to add outer nodes. ";
		failed_test_counter.Increment();
	}

	// test remove() functionality
	report += "Removing child node: ";
	ut::Array< ut::Tree<ut::String> >::Iterator leaf_iterator = tree.BeginLeaves();
	tree.Remove(leaf_iterator);

	// check the number of nodes after all manipulations
	report += "Calculating tree nodes: ";
	if (tree.Count() == 9)
	{
		report += "Success, ";
		report += ut::Print<ut::uint32>((ut::uint32)tree.Count());
		report += " nodes.";
	}
	else
	{
		report += "Fail, ";
		report += ut::Print<ut::uint32>((ut::uint32)tree.Count());
		report += " nodes.";
		failed_test_counter.Increment();
	}

	// test insert() functionality
	report += " Inserting nodes: ";
	if (tree.Insert(1, "inserted0"))
	{
		ut::Array< ut::Tree<ut::String> >::ConstIterator insert_iter = tree.BeginLeaves();
		insert_iter += 2;
		if (tree.Insert(insert_iter, "inserted1"))
		{
			report += "Success. ";
		}
		else
		{
			report += "Fail. ";
		}
	}
	else
	{
		report += "Fail. ";
		failed_test_counter.Increment();
	}

	// debug output: all leaves
	report += " Leaves output (forward): ";
	ut::Tree<ut::String>::Iterator forward_iter;
	for (forward_iter = tree.Begin(ut::iterator::Position::first);
	     forward_iter != tree.End(ut::iterator::Position::last);
	     ++forward_iter)
	{
		forward_iter->data += "!";
		report += forward_iter->data;
		report += " ";
	}

	// debug output: all leaves (reverse order)
	report += " Leaves output (backward): ";
	ut::Tree<ut::String>::ConstIterator back_iter;
	ut::Tree<ut::String>& cnode = tree;
	for (back_iter = cnode.Begin(ut::iterator::Position::last);
	     back_iter != cnode.End(ut::iterator::Position::first);
	     --back_iter)
	{
		report += back_iter->data;
		report += " ";
	}
}

//----------------------------------------------------------------------------//

AVLTreeTask::AVLTreeTask() : TestTask("AVL Tree")
{ }

void AVLTreeTask::Execute()
{
	// avltree test, following calls can cause
	// throwing an error if something went wrong:
	ut::AVLTree<int, ut::String> tree;
	tree.Insert(24, "__24");
	tree.Insert(2,  "__2");
	tree.Insert(32, "__32");
	tree.Insert(10, "__10");
	tree.Insert(15, "__15");
	tree.Insert(1,  "__1");
	tree.Insert(55, "__55");
	tree.Insert(4,  "__4");
	tree.Insert(127, "__127");
	tree.Insert(3,  "__3");
	tree.Insert(5,  "__5");
	tree.Insert(43, "__43");
	tree.Insert(60, "__60");
	tree.Insert(18, "__18");
	tree.Insert(22, "__22");
	tree.Insert(49, "__49");
	tree.Insert(29, "__29");
	tree.Insert(27, "__27");
	tree.Insert(25, "__25");

	// try to remove a leaf by key
	tree.Remove(32);
	tree.Remove(127);
	tree.Remove(3);
	tree.Remove(5);
	tree.Remove(24);

	// then insert some new values
	tree.Insert(26, "__26");
	tree.Insert(33, "__33");
	tree.Insert(128, "__128");
	tree.Insert(129, "__129");
	tree.Insert(26, "__26_1");
	tree.Insert(26, "__26_2");

	// try to find a specific value by key
	report += "searching element by key \'3\'(should be \'__3\'): ";
	ut::Optional<ut::String&> find_result = tree.Find(128);
	if (find_result)
	{
		ut::String& str = find_result.Get();
		report += str;
		if (str == "__128")
		{
			report += ". Success";
		}
		else
		{
			report += ". Fail";
			failed_test_counter.Increment();
		}
	}
	else
	{
		report += ut::String("failed to find element\n");
		failed_test_counter.Increment();
		return;
	}

	// iterate avl container forward
	report += ut::CRet() + "iterating forward: ";
	ut::AVLTree<int, ut::String>::ConstIterator riterator;
	int previous_key = 0;
	for (riterator = tree.Begin(ut::iterator::Position::first);
	     riterator != tree.End(ut::iterator::Position::last);
	     ++riterator)
	{
		const ut::Pair<const int, ut::String>& node = *riterator;
		report += node.second;

		int key = node.GetFirst();
		if (previous_key > key)
		{
			report += ut::String(" Error, invalid order!\n");
			failed_test_counter.Increment();
			return;
		}
		previous_key = key;
	}

	// iterate avl container backward
	report += ut::CRet() + "iterating backward: ";
	ut::AVLTree<int, ut::String>::Iterator literator;
	previous_key = 100000;
	for (literator = tree.Begin(ut::iterator::Position::last);
	     literator != tree.End(ut::iterator::Position::first);
	     --literator)
	{
		ut::Pair<const int, ut::String>& node = *literator;
		report += node.second;

		int key = node.GetFirst();
		if (previous_key < key)
		{
			report += ut::String(" Error, invalid order!\n");
			failed_test_counter.Increment();
			return;
		}
		previous_key = key;
	}

	// range based for loop
	report += ut::CRet() + ". Range based for loop:";
	previous_key = 0;
	for (auto inode : tree)
	{
		report += inode.second;

		int key = inode.GetFirst();
		if (previous_key > key)
		{
			report += ut::String(" Error, invalid order!\n");
			failed_test_counter.Increment();
			return;
		}
		previous_key = key;
	}

	// copy test
	report += ut::CRet() + "copy constructor: ";
	ut::AVLTree<int, ut::String> tree_copy0(tree);
	ut::Optional<ut::String&> copy0_find_result = tree_copy0.Find(128);
	if (copy0_find_result)
	{
		ut::String& str = copy0_find_result.Get();
		if (str == "__128")
		{
			report += " Success";
		}
		else
		{
			report += ". Fail";
			failed_test_counter.Increment();
		}
	}
	else
	{
		report += ut::String("failed to find element\n");
		failed_test_counter.Increment();
		return;
	}

	// copy assignment test
	report += ut::CRet() + "copy assignment: ";
	ut::AVLTree<int, ut::String> tree_copy1;
	tree_copy1 = tree;
	ut::Optional<ut::String&> copy1_find_result = tree_copy1.Find(128);
	if (copy1_find_result)
	{
		ut::String& str = copy1_find_result.Get();
		if (str == "__128")
		{
			report += " Success";
		}
		else
		{
			report += ". Fail";
			failed_test_counter.Increment();
		}
	}
	else
	{
		report += ut::String("failed to find element\n");
		failed_test_counter.Increment();
		return;
	}

	// move constructor test
	report += ut::CRet() + "move constructor: ";
	ut::AVLTree<int, ut::String> tree_move(ut::Move(tree));
	ut::Optional<ut::String&> move0_find_result = tree_move.Find(128);
	if (move0_find_result)
	{
		ut::String& str = move0_find_result.Get();
		if (str == "__128")
		{
			if (tree.Find(128))
			{
				report += ". Failed, value was copied, but not moved.";
			}
			else
			{
				report += " Success";
			}
		}
		else
		{
			report += ". Fail";
			failed_test_counter.Increment();
		}
	}
	else
	{
		report += ut::String("failed to find element\n");
		failed_test_counter.Increment();
		return;
	}

	// move assignment test
	report += ut::CRet() + "move assignment: ";
	tree = ut::Move(tree_copy0);
	ut::Optional<ut::String&> move1_find_result = tree.Find(128);
	if (move1_find_result)
	{
		ut::String& str = move1_find_result.Get();
		if (str == "__128")
		{
			if (tree_copy0.Find(128))
			{
				report += ". Failed, value was copied, but not moved.";
			}
			else
			{
				report += " Success";
			}
		}
		else
		{
			report += ". Fail";
			failed_test_counter.Increment();
		}
	}
	else
	{
		report += ut::String("failed to find element\n");
		failed_test_counter.Increment();
		return;
	}

	// test with non-default allocator
	report += ut::CRet() + "non-default allocator: ";
	ut::AVLTree<int, ut::String, TestAllocator> al_tree_0;
	ut::AVLTree<int, ut::String, TestAllocator> al_tree_1;
	ut::AVLTree<int, ut::String, TestAllocator> al_tree_2;
	al_tree_0.Insert(49, "__49");
	al_tree_0.Insert(29, "__29");
	al_tree_0.Insert(27, "__27");
	al_tree_0.Insert(25, "__25");
	al_tree_0.Remove(27);
	al_tree_0.Insert(26, "__26");

	al_tree_1 = al_tree_0;
	al_tree_2 = ut::Move(al_tree_1);

	find_result = al_tree_2.Find(29);
	if (find_result)
	{
		ut::String& str = find_result.Get();
		if (str == "__29")
		{
			report += " Success";
		}
		else
		{
			report += " fail";
			failed_test_counter.Increment();
		}
	}
	else
	{
		report += ut::String("failed to find element.");
		failed_test_counter.Increment();
		return;
	}

	ut::AVLTree<int, MapValue> perf_tree;
	ut::time::Counter counter;
	ut::Array<int> source = GenerateMapArray(perf_arr_count);
	const size_t source_count = source.Count();
	report += ut::cret + ut::String("Insert: ");
	counter.Start();
	for (size_t i = 0; i < source_count; i++)
	{
		MapValue val;
		val.ival = source[i];

		perf_tree.Insert(source[i], ut::Move(val));
	}
	double time = counter.GetTime();
	report += ut::Print(time) + "ms. ";

	report += ut::String("Iteration: ");
	counter.Start();
	ut::AVLTree<int, MapValue>::Iterator perf_it;
	for (perf_it = perf_tree.Begin(ut::iterator::Position::first);
	     perf_it != perf_tree.End(ut::iterator::Position::last);
	     perf_it++)
	{
		ut::Pair<const int, MapValue>& node = *perf_it;
		node.second.ival++;
		if (node.second.ival != -1)
		{
			node.second.ival--;
		}
	}
	time = counter.GetTime();
	report += ut::Print(time) + "ms. ";

	report += ut::String("Search: ");
	counter.Start();
	for (size_t i = 0; i < source_count; i++)
	{
		const int key = source[i];
		ut::Optional<MapValue&> element = perf_tree.Find(key);
		if (!element || element->ival != key)
		{
			report += ut::String("FAILED! Element ") + ut::Print(key) + " is invalid or was not found.";
			failed_test_counter.Increment();
			return;
		}
	}
	time = counter.GetTime();
	report += ut::Print(time) + "ms. ";
}

//----------------------------------------------------------------------------//

template<class IntHashMapType, class StringHashMapType, class IntHashMapHashMapType>
ut::String HashMapTest()
{
	ut::String report;

	ut::time::Counter counter;
	IntHashMapType map;
	StringHashMapType str_map;
	std::unordered_map<int, MapValue> std_map;

	ut::Array<int> source = GenerateMapArray(TestTask::perf_arr_count);
	report += ut::String("Elements: ") + ut::Print(source.Count()) + ". ";

	const size_t test_element_id = source.Count() / 2;

	report += ut::String("Insert (int): ");
	counter.Start();
	for (size_t i = 0; i < source.Count(); i++)
	{
		MapValue val;
		val.ival = source[i];

		map.Insert(source[i], ut::Move(val));
	}
	double time = counter.GetTime();
	report += ut::Print(time) + "ms. ";

	report += ut::String("Insert (str): ");
	counter.Start();
	for (size_t i = 0; i < source.Count(); i++)
	{
		MapValue val;
		val.ival = source[i];

		str_map.Insert(ut::String("_____") + ut::Print(source[i]), ut::Move(val));
	}
	time = counter.GetTime();
	report += ut::Print(time) + "ms. ";

	report += ut::String("Insert (std int): ");
	counter.Start();
	for (size_t i = 0; i < source.Count(); i++)
	{
		MapValue val;
		val.ival = source[i];

		std_map.insert(std::pair<int, MapValue>(source[i], std::move(val)));
	}
	time = counter.GetTime();
	report += ut::Print(time) + "ms. ";

	const size_t element_count = map.Count();
	if (element_count != source.Count())
	{
		report += ut::String("FAILED! Invalid element count (") + ut::Print(element_count) + ")\n";
		failed_test_counter.Increment();
		return report;
	}

	report += ut::String("Collision(int): ") + ut::Print(map.GetCollisionCount()) + ". ";
	report += ut::String("Collision(str): ") + ut::Print(str_map.GetCollisionCount()) + ". ";

	report += ut::String("Iteration (iterator): ");
	counter.Start();
	typename IntHashMapType::Iterator iterator;
	size_t it_count = 0;
	for (iterator = map.Begin(); iterator != map.End(); ++iterator)
	{
		ut::Pair<const int, MapValue>& element = *iterator;
		element.second.ival++;
		if (iterator->second.ival != -1)
		{
			iterator->second.ival--;
		}
		it_count++;
	}
	time = counter.GetTime();
	report += ut::Print(time) + "ms. ";
	if (it_count != map.Count())
	{
		report += ut::String("FAILED! Iterated less elements (") + ut::Print(it_count) + ") than the map size.";
		failed_test_counter.Increment();
		return report;
	}


	typename IntHashMapType::ConstIterator riterator;
	int itertesta = 0;
	for (riterator = map.Begin(); riterator != map.End(); ++riterator)
	{
		const ut::Pair<const int, MapValue>& element = *riterator;
		itertesta = element.second.ival + riterator->second.ival;
	}

	report += ut::String("Iteration (std): ");
	counter.Start();
	it_count = 0;
	std::unordered_map<int, MapValue>::iterator std_iterator;
	for (std_iterator = std_map.begin(); std_iterator != std_map.end(); ++std_iterator)
	{
		std::pair<const int, MapValue>& element = *std_iterator;
		element.second.ival++;
		if (element.second.ival != -1)
		{
			element.second.ival--;
		}
		it_count++;
	}
	time = counter.GetTime();
	report += ut::Print(time) + "ms. ";

	report += ut::String("Search (int): ");
	counter.Start();
	for (size_t i = 0; i < element_count; i++)
	{
		const int key = source[i];
		ut::Optional<MapValue&> element = map.Find(key);
		if (!element || element->ival != key)
		{
			report += ut::String("FAILED! Element ") + ut::Print(key) + " is invalid or was not found.";
			failed_test_counter.Increment();
			return report;
		}
	}
	time = counter.GetTime();
	report += ut::Print(time) + "ms. ";

	report += ut::String("Search (std): ");
	counter.Start();
	for (size_t i = 0; i < element_count; i++)
	{
		const int key = source[i];
		std::unordered_map<int, MapValue>::iterator element = std_map.find(key);
		if (element == std_map.end() || element->second.ival != key)
		{
			report += ut::String("FAILED! Element ") + ut::Print(key) + " is invalid or was not found.";
			failed_test_counter.Increment();
			return report;
		}
	}
	time = counter.GetTime();
	report += ut::Print(time) + "ms. ";

	report += ut::String("Search (str): ");
	counter.Start();
	for (size_t i = 0; i < element_count; i++)
	{
		const ut::String key = ut::String("_____") + ut::Print(source[i]);
		ut::Optional<MapValue&> element = str_map.Find(key);
		if (!element || element->ival != source[i])
		{
			report += ut::String("FAILED! Element ") + ut::Print(key) + " is invalid or was not found (str).";
			failed_test_counter.Increment();
			return report;
		}
	}
	time = counter.GetTime();
	report += ut::Print(time) + "ms. ";

	// check constness
	const IntHashMapType& map_cref = map;
	ut::Optional<const MapValue&> fcref = map_cref.Find(4);

	// check copy constructor
	IntHashMapType map_copy(map);
	if (map_copy.Count() == map.Count())
	{
		ut::Optional<MapValue&> fcres = map_copy.Find(source[test_element_id]);
		if (fcres)
		{
			if (fcres->ival != source[test_element_id])
			{
				report += ut::String("FAILED! Copy constructor - invalid element(") +
					ut::Print(fcres.Get()) + "), must be " + ut::Print(source[test_element_id]);
				failed_test_counter.Increment();
				return report;
			}
		}
		else
		{
			report += "FAILED! Copy constructor - failed to find element.";
			failed_test_counter.Increment();
			return report;
		}
	}
	else
	{
		report += "FAILED! Copy constructor - invalid element count.";
		failed_test_counter.Increment();
		return report;
	}

	// check Reset
	map.Reset();
	if (map.Find(source[test_element_id]) || map.Count() != 0)
	{
		report += "FAILED! Reset() failed.";
		failed_test_counter.Increment();
		return report;
	}

	// check copy operator
	map = map_copy;
	if (map.Count() == map_copy.Count())
	{
		ut::Optional<MapValue&> fcres = map.Find(source[test_element_id]);
		if (fcres)
		{
			if (fcres->ival != source[test_element_id])
			{
				report += ut::String("FAILED! Copy operator - invalid element(") +
					ut::Print(fcres.Get()) + "), must be " + ut::Print(source[test_element_id]);
				failed_test_counter.Increment();
				return report;
			}
		}
		else
		{
			report += "FAILED! Copy operator - failed to find element.";
			failed_test_counter.Increment();
			return report;
		}
	}
	else
	{
		report += "FAILED! Copy operator - invalid element count.";
		failed_test_counter.Increment();
		return report;
	}

	// check move operator
	map = ut::Move(map_copy);
	if (map_copy.Find(source[test_element_id]) || map_copy.Count() != 0)
	{
		report += "FAILED! Original map was not destroyed after the move operation.";
		failed_test_counter.Increment();
		return report;
	}
	if (map.Count() == source.Count())
	{
		ut::Optional<MapValue&> fcres = map.Find(source[test_element_id]);
		if (fcres)
		{
			if (fcres->ival != source[test_element_id])
			{
				report += ut::String("FAILED! Move operator - invalid element(") +
					ut::Print(fcres.Get()) + "), must be " + ut::Print(source[test_element_id]);
				failed_test_counter.Increment();
				return report;
			}
		}
		else
		{
			report += "FAILED! Move operator - failed to find element.";
			failed_test_counter.Increment();
			return report;
		}
	}
	else
	{
		report += "FAILED! Move operator - invalid element count.";
		failed_test_counter.Increment();
		return report;
	}

	// check move constructor
	IntHashMapType map_move = ut::Move(map);
	if (map.Find(source[test_element_id]) || map.Count() != 0)
	{
		report += "FAILED! Original map was not destroyed after the move construction.";
		failed_test_counter.Increment();
		return report;
	}
	if (map_move.Count() == source.Count())
	{
		ut::Optional<MapValue&> fcres = map_move.Find(source[test_element_id]);
		if (fcres)
		{
			if (fcres->ival != source[test_element_id])
			{
				report += ut::String("FAILED! Move constructor - invalid element(") +
					ut::Print(fcres.Get()) + "), must be " + ut::Print(source[test_element_id]);
				failed_test_counter.Increment();
				return report;
			}
		}
		else
		{
			report += "FAILED! Move constructor - failed to find element.";
			failed_test_counter.Increment();
			return report;
		}
	}
	else
	{
		report += "FAILED! Move constructor - invalid element count.";
		failed_test_counter.Increment();
		return report;
	}

	// check remove
	if (!map_move.Remove(source[test_element_id]))
	{
		report += "FAILED! Element to be removed was not found.";
		failed_test_counter.Increment();
		return report;
	}
	if (map_move.Count() != source.Count() - 1)
	{
		report += "FAILED! Invalid element count after Remove() call.";
		failed_test_counter.Increment();
		return report;
	}
	for (size_t i = 0; i < source.Count(); i++)
	{
		const int key = source[i];
		ut::Optional<MapValue&> element = map_move.Find(key);

		if (i == test_element_id)
		{
			if (element)
			{
				ut::Optional<MapValue&> element = map_move.Find(key);
				report += ut::String("FAILED! Found element that must be deleted after Remove(") +
					ut::Print(source[test_element_id]) + ") call.";
				failed_test_counter.Increment();
				return report;
			}
		}
		else
		{
			if (!element)
			{
				report += ut::String("FAILED! Element ") + ut::Print(key) +
					" was not found after Remove(" +
					ut::Print(source[test_element_id]) + ") call.";

				report += ut::Print(i) + " " + ut::Print(test_element_id);
				failed_test_counter.Increment();
				return report;
			}

			if (element->ival != key)
			{
				report += ut::String("FAILED! Element ") + ut::Print(key) +
					" is invalid after Remove(" +
					ut::Print(source[test_element_id]) + ") call (" +
					ut::Print(element->ival) + ").";
				failed_test_counter.Increment();
				return report;
			}
		}
	}
	it_count = 0;
	for (riterator = map_move.Begin(); riterator != map_move.End(); ++riterator)
	{
		const ut::Pair<const int, MapValue>& element = *riterator;
		it_count++;
	}
	if (it_count != map_move.Count())
	{
		report += ut::String("FAILED! Iterated less elements (") + ut::Print(it_count) + ") than the map size after Remove().";
		failed_test_counter.Increment();
		return report;
	}

	// test zero sized map
	IntHashMapType zmap;
	for (iterator = zmap.Begin(); iterator != zmap.End(); ++iterator)
	{
		ut::Pair<const int, MapValue>& element = *iterator;
		element.second.ival++;
	}

	// check map of the maps behaviour
	report += ut::String("Testing recursive map ");
	IntHashMapType tmap;
	for (size_t i = 0; i < source.Count(); i++)
	{
		MapValue val;
		val.ival = source[i];
		tmap.Insert(source[i], ut::Move(val));
	}
	IntHashMapHashMapType map2;
	map2.Insert(20, IntHashMapType());
	map2.Insert(10, ut::Move(tmap));
	map2.Insert(0, IntHashMapType());
	for (int i = 0; i < 100; i++)
	{
		map2.Insert(i, IntHashMapType());
	}

	ut::Optional<IntHashMapType&> res0 = map2.Find(0);
	ut::Optional<IntHashMapType&> res1 = map2.Find(10);
	ut::Optional<IntHashMapType&> res2 = map2.Find(20);
	if (!res0 || !res2 || res0->Count() != 0 || res2->Count() != 0)
	{
		report += ut::String("FAILED! Invalid map (map) element.");
		failed_test_counter.Increment();
		return report;
	}

	if (res1)
	{
		if (res1->Count() != source.Count())
		{
			report += ut::String("FAILED! Invalid map(map) element count (") + ut::Print(res1->Count()) + ")\n";
		}

		for (size_t i = 0; i < res1->Count(); i++)
		{
			const int key = source[i];
			ut::Optional<MapValue&> element = res1->Find(key);
			if (!element || element->ival != key)
			{
				report += ut::String("FAILED! Map(map) element ") + ut::Print(key) + " is invalid or was not found.";
				failed_test_counter.Increment();
				return report;
			}
		}

	}
	else
	{
		report += ut::String("FAILED! Cannot find the desired element in the map(map) object.");
		failed_test_counter.Increment();
		return report;
	}
	report += ut::String("(ok).\n");

	report += ut::String("Iteration (range-based for loop): ");
	const IntHashMapType& cmapref = map;
	for (const ut::Pair<const int, MapValue>& e : cmapref)
	{
		report += ut::Print(e.GetSecond().ival) + " ";
	}

	// success
	return report;
}

HashmapTask::HashmapTask() : TestTask("Hashmap")
{ }

void HashmapTask::Execute()
{
	report += ut::String("Dense:") + ut::cret;
	report += HashMapTest<ut::DenseHashMap<int, MapValue>,
	                      ut::DenseHashMap<ut::String, MapValue>,
	                      ut::DenseHashMap<int, ut::DenseHashMap<int, MapValue> > >();

	report += ut::String("Sparse:") + ut::cret;
	report += HashMapTest<ut::SparseHashMap<int, MapValue>,
	                      ut::SparseHashMap<ut::String, MapValue>,
	                      ut::SparseHashMap<int, ut::SparseHashMap<int, MapValue> > >();
}

//----------------------------------------------------------------------------//

SharedPtrTask::SharedPtrTask() : TestTask("Shared pointer")
{ }

void SharedPtrTask::Execute()
{
	static const ut::thread_safety::Mode safety_mode = ut::thread_safety::Mode::on;
	ut::WeakPtr<int, safety_mode> weak;

	if (weak.IsValid()) // can't be valid here
	{
		report += "Fail(00)";
		failed_test_counter.Increment();
		return;
	}

	if(true)
	{
		ut::SharedPtr<int, safety_mode> ptr(ut::MakeShared<int>(-1));
		ut::SharedPtr<int, safety_mode> alt_ptr(ut::MakeShared<int>(2));

		ut::SharedPtr<int, safety_mode> ptr0(ptr);

		ptr = alt_ptr;

		ut::WeakPtr<int, safety_mode> weak0(ptr);

		weak0 = alt_ptr;

		weak = weak0;

		if (!weak.IsValid()) // must be valid here
		{
			report += "Fail(01)";
			failed_test_counter.Increment();
		}

		ut::SharedPtr<int, safety_mode> ptr_convertion = weak.Pin();
	}

	if (weak.IsValid()) // can't be valid here
	{
		report += "Fail(02)";
		failed_test_counter.Increment();
		return;
	}

	report += "reached the end. Ok!";
}

//----------------------------------------------------------------------------//
typedef ut::Tuple<int, char&, ut::UniquePtr<int>, ut::SharedPtr<ut::uint>, ut::Array<ut::byte> > TestTuple;

TupleTask::TupleTask() : TestTask("Tuple template")
{ }

void TupleTask::Execute()
{
	char b = 3;
	ut::Array<ut::byte> arr;
	arr.Add(0);
	arr.Add(1);
	arr.Add(2);

	ut::UniquePtr<int> uniq_ptr(ut::MakeUnique<int>(10));
	ut::SharedPtr<ut::uint> sh_ptr(ut::MakeShared<ut::uint>(12));

	TestTuple c(0, b, Move(uniq_ptr), sh_ptr, Move(arr));

	TestTuple::Item<2>::Type test_unique_ptr(ut::MakeUnique<int>(1));

	const int n = TestTuple::size;
	if (n != 5)
	{
		report += "Fail(count)";
		failed_test_counter.Increment();
		return;
	}

	if (c.Get<0>() != 0)
	{
		report += "Fail(0)";
		failed_test_counter.Increment();
		return;
	}

	c.Get<1>() = 11;
	if (b != 11)
	{
		report += "Fail(1)";
		failed_test_counter.Increment();
		return;
	}

	if (c.Get<2>().GetRef() != 10)
	{
		report += "Fail(2)";
		failed_test_counter.Increment();
		return;
	}

	if (c.Get<3>().GetRef() != 12)
	{
		report += "Fail(3)";
		failed_test_counter.Increment();
		return;
	}

	if (c.Get<4>().Count() != 3 || c.Get<4>()[0] != 0 || c.Get<4>()[1] != 1 || c.Get<4>()[2] != 2)
	{
		report += "Fail(4)";
		failed_test_counter.Increment();
		return;
	}

	// compility check
	c.Get<int>() = 2;
	c.Get<char&>() = b;
	c.Get< ut::Array<ut::byte> >() = ut::Array<ut::byte>();
	char& c_test = c.Get<char&>();
	c_test = c.Get<1>();
	const TestTuple& rc = c;
	int i_test = rc.Get<0>();
	const char& cc_test = rc.Get<1>();
	const int& ri_test = rc.Get<int>();

	report += "success";
}

//----------------------------------------------------------------------------//
OptionalTask::OptionalTask() : TestTask("ut::Optional")
{ }

void OptionalTask::Execute()
{
	ut::String test_str("test");

	ut::Optional<ut::String&> opt_str(test_str);

	if (opt_str.Get() != test_str)
	{
		report += "fail! references don't match.";
		failed_test_counter.Increment();
		return;
	}

	// move constructor
	ut::String test_str1(opt_str.Move());
	if (test_str1 != "test")
	{
		report += "fail! while copying to string.";
		failed_test_counter.Increment();
		return;
	}
	else
	{
		if (test_str.Length() > 0)
		{
			report += "fail! string was copied instead of being moved.";
			failed_test_counter.Increment();
			return;
		}
	}

	// move assignment
	test_str = "modified_str";
	ut::Optional<ut::String&> opt_str1(test_str1);
	opt_str = ut::Move(opt_str1);
	if (opt_str.Get() != "test")
	{
		report += "fail! ut::optional wasn't moved properly.";
		failed_test_counter.Increment();
		return;
	}

	// assignment
	opt_str = ut::Optional<ut::String&>(test_str);
	if (opt_str.Get() != "modified_str")
	{
		report += "fail! ut::optional must rebind reference on assignment.";
		failed_test_counter.Increment();
		return;
	}

	// const
	const ut::Optional<const ut::String&> opt_str_const(test_str1);
	const ut::String& const_test = opt_str_const.Get();

	report += "success";
}

//----------------------------------------------------------------------------//
ResultTask::ResultTask() : TestTask("ut::Result")
{ }

void ResultTask::Execute()
{
	ut::String test_str = "test";
	const ut::String& test_const_str = "const";

	ut::Result<ut::String&, int> test_result(test_str);
	if (test_result.Get() != test_str)
	{
		report += "fail! references don't match.";
		failed_test_counter.Increment();
		return;
	}

	ut::String& str_ref = test_result.Get();
	str_ref += "+++";

	ut::String test_move_str;
	test_move_str = test_result.Move();
	if (test_str.Length() != 0)
	{
		report += "fail! string was copied instead of being moved.";
		failed_test_counter.Increment();
		return;
	}

	ut::Result<const ut::String&, int> const_result(test_const_str);
	ut::String test_copy_str = const_result.Get();

	ut::Result<const ut::String&, int> const_result_alt(ut::MakeAlt<int>(10));

	ut::Result<ut::String, int> def_result;
	if (!def_result)
	{
		report += "fail! default result has no value.";
		failed_test_counter.Increment();
		return;
	}
	else if (def_result.Get().Length() != 0)
	{
		report += "fail! default result has corrupted value.";
		failed_test_counter.Increment();
		return;
	}

	report += "success";
}

//----------------------------------------------------------------------------//
PairTask::PairTask() : TestTask("ut::Pair")
{ }

void PairTask::Execute()
{
	ut::String test_str_0 = "test";
	ut::String test_str_1 = "test";
	const ut::String& test_const_str = "const";

	ut::Pair<ut::String&, ut::String> pair_0(test_str_0, "pair");
	ut::Pair<ut::String&, ut::String> pair_1(Move(pair_0));

	if (pair_0.second.Length() != 0)
	{
		report += "fail! string was copied instead of being moved.";
		failed_test_counter.Increment();
		return;
	}

	ut::Pair<int, const ut::String&> const_pair(24, test_const_str);

	ut::String test_str = "test";
	ut::Pair<ut::String&, const ut::String&> ref_pair(test_str, test_const_str);

	report += "success";
}

//----------------------------------------------------------------------------//
SmartPtrTask::SmartPtrTask() : TestTask("Smart pointers")
{ }

struct ContainerTestBase
{
	int a;
};

struct ContainerTestDerived : public ContainerTestBase
{
	int b;
};

struct ContainerTestAnother
{
	int a;
};

void SmartPtrTask::Execute()
{
	// must be compilable
	ut::UniquePtr<ContainerTestAnother> other = ut::MakeUnique<ContainerTestAnother>();
	ut::UniquePtr<ContainerTestDerived> derived = ut::MakeUnique<ContainerTestDerived>();
	ut::UniquePtr<ContainerTestBase> base(ut::Move(derived));
	base = ut::Move(derived);

	// must be non-compilable
	//ut::UniquePtr<TestBase> base2(ut::Move(other));
	//base = ut::Move(other);

	// must be compilable
	ut::SharedPtr<ContainerTestAnother, ut::thread_safety::Mode::on> sh_other_safe = ut::MakeSafeShared<ContainerTestAnother>();
	ut::SharedPtr<ContainerTestDerived, ut::thread_safety::Mode::on> sh_derived_safe = ut::MakeSafeShared<ContainerTestDerived>();
	ut::SharedPtr<ContainerTestBase, ut::thread_safety::Mode::on> sh_base_safe(ut::Move(sh_derived_safe));
	sh_base_safe = ut::Move(sh_derived_safe);

	ut::SharedPtr<ContainerTestAnother, ut::thread_safety::Mode::off> sh_other_unsafe = ut::MakeUnsafeShared<ContainerTestAnother>();
	ut::SharedPtr<ContainerTestDerived, ut::thread_safety::Mode::off> sh_derived_unsafe = ut::MakeUnsafeShared<ContainerTestDerived>();
	ut::SharedPtr<ContainerTestBase, ut::thread_safety::Mode::off> sh_base_unsafe(ut::Move(sh_derived_unsafe));
	sh_base_unsafe = ut::Move(sh_derived_unsafe);

	// must be non-compilable
	/*
	ut::SharedPtr<TestBase, ut::thread_safety::Mode::on> sh_base2_safe(ut::Move(sh_other_safe));
	sh_base_safe = ut::Move(sh_other_safe);
	ut::SharedPtr<TestBase, ut::thread_safety::Mode::off> sh_base2_unsafe(ut::Move(sh_other_unsafe));
	sh_base_unsafe = ut::Move(sh_other_unsafe);
	sh_base_safe = sh_base_unsafe;
	*/

	// must be compilable
	ut::WeakPtr<ContainerTestAnother, ut::thread_safety::Mode::on> wk_other_safe;
	ut::WeakPtr<ContainerTestDerived, ut::thread_safety::Mode::on> wk_derived_safe;
	ut::WeakPtr<ContainerTestBase, ut::thread_safety::Mode::on> wk_base_safe = wk_derived_safe;
	wk_base_safe = ut::Move(wk_derived_safe);
	wk_base_safe = sh_derived_safe;

	ut::WeakPtr<ContainerTestAnother, ut::thread_safety::Mode::off> wk_other_unsafe;
	ut::WeakPtr<ContainerTestDerived, ut::thread_safety::Mode::off> wk_derived_unsafe;
	ut::WeakPtr<ContainerTestBase, ut::thread_safety::Mode::off> wk_base_unsafe = wk_derived_unsafe;
	wk_base_unsafe = ut::Move(wk_derived_unsafe);
	wk_base_unsafe = sh_derived_unsafe;

	// must be non-compilable
	/*
	ut::WeakPtr<TestBase, ut::thread_safety::Mode::on> wk_base2_safe(ut::Move(wk_other_safe));
	wk_base_safe = ut::Move(sh_other_safe);
	ut::WeakPtr<TestBase, ut::thread_safety::Mode::off> wk_base2_unsafe(ut::Move(wk_other_unsafe));
	wk_base_unsafe = ut::Move(sh_other_unsafe);
	wk_base_safe = wk_base_unsafe;
	wk_base_safe = sh_derived_unsafe;
	*/

	report += "success";
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//

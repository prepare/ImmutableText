#include<typeinfo>
#include<string.h>
#include <string>
#include <iostream>

using namespace std;

// <summary>Holds the default size for primitive blocks of characters.</summary>
const int BLOCK_SIZE = 1 << 6;

/// <summary>Holds the mask used to ensure a block boundary cesures.</summary>
const int BLOCK_MASK = ~(BLOCK_SIZE - 1);

class Node {
public:
		virtual int Length() = 0;
		virtual Node* SubNode(int start, int end) = 0;
		virtual wchar_t GetCharAt(int index) = 0;
		virtual void CopyTo (int sourceIndex, wchar_t* destination, int destinationIndex, int count) = 0;
};

class WideLeafNode : public Node
{
private:
	int size;
	const wchar_t* data;
public:
	WideLeafNode(const wchar_t* data, int size)
	{
		this->data = data;
		this->size = size;
	}

	int Length() {
		return size;
	}

	virtual Node* SubNode(int start, int end)  {
		if (start == 0 && end == Length()) {
			return this;
		}
		wchar_t* subArray = new wchar_t[end - start];
		CopyTo(start, subArray, 0, end - start);
		return new WideLeafNode(subArray, end - start);
	}

	wchar_t GetCharAt(int index)
	{
		return *(data + index);
	}

	void CopyTo (int sourceIndex, wchar_t* destination, int destinationIndex, int count) {
		const wchar_t* src = data;
		wchar_t* ptr = destination + destinationIndex;
		wchar_t* end = ptr + count;
		while(ptr != end) {
			*ptr = *src;
			ptr++;src++;
		}
	}

	~WideLeafNode()
	{
		delete [] data;
	}
};

static Node* NodeOf (Node* node, int offset, int length);
static Node* ConcatNodes (Node* node1, Node* node2);

class CompositeNode : public Node
{
public:
	int count;
	Node* head;
	Node* tail;

	CompositeNode(int count, Node* head, Node* tail)
	{
		this->count = count;
		this->head = head;
		this->tail = tail;
	}

	int Length() {
		return count;
	}

	virtual Node* SubNode(int start, int end)  {
		int cesure = head->Length();
		if (end <= cesure) {
			return head->SubNode(start, end);
		}

		if (start >= cesure) {
		 return tail->SubNode(start - cesure, end - cesure);
	 }

		if (start == 0 && end == count)
			return this;
		// Overlaps head and tail.
		return ConcatNodes(head->SubNode(start, cesure), tail->SubNode(0, end - cesure));
	}

	wchar_t GetCharAt(int index)
	{
		int headLength = head->Length();
		if (index < headLength) {
			return head->GetCharAt(index);
		}
		return tail->GetCharAt(index - headLength);
	}

	void CopyTo (int sourceIndex, wchar_t* destination, int destinationIndex, int count) {
		int cesure = head->Length();
		if (sourceIndex + count <= cesure) {
			head->CopyTo(sourceIndex, destination, destinationIndex, count);
			return;
		}
		if (sourceIndex >= cesure) {
			tail->CopyTo(sourceIndex - cesure, destination, destinationIndex, count);
			return;
		}
		// Overlaps head and tail.
		int headChunkSize = cesure - sourceIndex;
		head->CopyTo(sourceIndex, destination, destinationIndex, headChunkSize);
		tail->CopyTo(0, destination, destinationIndex + headChunkSize, count - headChunkSize);
	}

	~CompositeNode()
	{
		delete head;
		delete tail;
	}

	CompositeNode* RotateRight()
	{
		if (typeid(*head) != typeid (CompositeNode))
			return this;
		CompositeNode* P = (CompositeNode*)head;
		Node* A = P->head;
	 	Node* B = P->tail;
	 	Node* C = this->tail;
		int tailLength = B->Length() + C->Length();
		return new CompositeNode(A->Length() + tailLength, A, new CompositeNode(tailLength, B, C ));
	}

	CompositeNode* RotateLeft()
	{
	 // See: http://en.wikipedia.org/wiki/Tree_rotation
	 if (typeid(*tail) != typeid (CompositeNode))
		 return this;

		 CompositeNode* Q = (CompositeNode*)tail;
	 Node* B = Q->head;
	 Node* C = Q->tail;
	 Node* A = this->head;
	 int headLength = A->Length() + B->Length();
	 return new CompositeNode (headLength +  C->Length(), new CompositeNode (headLength, A, B), C);
	}
};

static Node* ConcatNodes (Node* node1, Node* node2)
{
 // All Text instances are maintained balanced:
 //   (head < tail * 2) & (tail < head * 2)
 int length = node1->Length() + node2->Length();
 if (length <= BLOCK_SIZE) { // Merges to primitive.
	 wchar_t* mergedArray = new wchar_t[length];
	 node1->CopyTo (0, mergedArray, 0, node1->Length());
	 node2->CopyTo (0, mergedArray, node1->Length(), node2->Length());
	 return new WideLeafNode(mergedArray, length);
 }
 // Returns a composite.
 Node* head = node1;
 Node* tail = node2;
 CompositeNode* compositeTail = (CompositeNode*)tail;
 if (typeid(*tail) == typeid (CompositeNode) && (head->Length() << 1) < tail->Length()) {
	 // head too small, returns (head + tail/2) + (tail/2)
	 if (compositeTail->head->Length() > compositeTail->tail->Length()) {
		 // Rotates to concatenate with smaller part.
		 compositeTail = compositeTail->RotateRight ();
	 }
	 head = ConcatNodes (head, compositeTail->head);
	 tail = compositeTail->tail;
 } else {
	 if (typeid(*head) == typeid (CompositeNode)) {
		 CompositeNode* compositeHead = (CompositeNode*)head;
		 if ((tail->Length() << 1) < head->Length()) {
			 // tail too small, returns (head/2) + (head/2 concat tail)
			 if (compositeHead->tail->Length() > compositeHead->head->Length()) {
				 // Rotates to concatenate with smaller part.
				 compositeHead = compositeHead->RotateLeft ();
			 }
			 tail = ConcatNodes (compositeHead->tail, tail);
			 head = compositeHead->head;
		 }
	 }
 }
 return new CompositeNode (head->Length() + tail->Length(), head, tail);
}

static Node* NodeOf (Node* node, int offset, int length)
{
 if (length <= BLOCK_SIZE) {
	 return node->SubNode (offset, offset + length);
 }
 // Splits on a block boundary.
 int half = ((length + BLOCK_SIZE) >> 1) & BLOCK_MASK;
 Node* head = NodeOf (node, offset, half);
 Node* tail = NodeOf (node, offset + half, length - half);
 return new CompositeNode (head->Length() + tail->Length(), head, tail);
}

struct InnerLeaf
{
	Node* leafNode;
	int offset;
};

class ImmutableText
{
private:
	Node* root;
public:
	ImmutableText(Node* root)
	{
		this->root = root;
	}

	ImmutableText(wstring str)
	{
		wchar_t* runes = new wchar_t[str.length()];
		memcpy(runes, str.c_str(), str.length() * sizeof(wchar_t));
		this->root = new WideLeafNode(runes, str.length());
	}

	int Length()
	{
		return root->Length();
	}

	wchar_t GetCharAt(int index)
	{
		InnerLeaf leaf = FindLeaf(index, 0);
		return leaf.leafNode->GetCharAt(index - leaf.offset);
	}

	ImmutableText* EnsureChunked()
	{
		int len = Length();
		if (typeid(*root) != typeid (CompositeNode) || len > BLOCK_SIZE)
			return new ImmutableText(NodeOf (root, 0, len));
		return this;
	}

	InnerLeaf FindLeaf(int index, int offset)
	{
		InnerLeaf result;
		result.leafNode = 0;
		result.offset = -1;
		Node* node = this->root;
		while (1) {
			if (index >= node->Length()) {
				return result;
			}
			if (typeid(*node) == typeid (CompositeNode)) {
				CompositeNode* composite = (CompositeNode*)node;
				if (index < composite->head->Length()) {
					node = composite->head;
				} else {
					offset += composite->head->Length();
					index -= composite->head->Length();
					node = composite->tail;
				}
				continue;
			}
			result.leafNode = node;
			result.offset = offset;
			return result;
		}
	}

	/// <summary>
	/// Concatenates the specified text to the end of this text.
	/// This method is very fast (faster even than
	/// <code>StringBuffer.append(String)</code>) and still returns
	/// a text instance with an internal binary tree of minimal depth!
	/// </summary>
	/// <param name="that">that the text that is concatenated.</param>
	/// <returns><code>this + that</code></returns>
	ImmutableText* Concat(ImmutableText* that)
	{
		if (that->Length() == 0) {
			return this;
		}
		if (this->Length() == 0) {
			return that;
		}
		return new ImmutableText (ConcatNodes (this->EnsureChunked()->root, that->EnsureChunked()->root));
	}

	/// <summary>
	/// Returns a portion of this text.
	// </summary>
	/// <returns>the sub-text starting at the specified start position and ending just before the specified end position.</returns>
	ImmutableText* GetText(int start, int count)
	{
		int end = start + count;
		if (start == 0 && end == Length()) {
			return this;
		}
		if (start == end) {
			return new ImmutableText (new WideLeafNode (new wchar_t[0], 0));
		}
		return new ImmutableText (this->root->SubNode (start, end));
	}

	ImmutableText* InsertText(int index, ImmutableText* text)
	 {
		 // TODO: Fix memory leaking - since it's possible that these methods return 'this' delete always will not work here.
		return this->GetText (0, index)->Concat (text)->Concat (this->SubText (index));
	}
	/*
	func (this ImmutableText) InsertString(index int, text string) ImmutableText {
		return this.InsertText (index, CreateImmutableText (text))
	}

	func CreateImmutableText (text string) ImmutableText {
		return ImmutableText { WideLeafNode { []rune(text) } }
	}

	*/

	/// <summary>
	/// Returns the text without the characters between the specified indexes.
	/// </summary>
	/// <returns><code>subtext(0, start).concat(subtext(end))</code></returns>
	ImmutableText* RemoveText(int start, int count)
	{
		if (count == 0)
			return this;
		int end = start + count;
	//	if (end > Length)
	//		throw new IndexOutOfRangeException ();
		// TODO: Fix memory leaking - since it's possible that these methods return 'this' delete always will not work here.
		return this->EnsureChunked ()->GetText (0, start)->Concat (this->SubText (end));
	}

	ImmutableText* SubText(int start)
	{
		return this->GetText (start, this->Length() - start);
	}
	wstring ToString()
	{
		wchar_t* runes = new wchar_t[this->Length()];
		this->root->CopyTo(0, runes, 0, this->Length());
		return wstring(runes);
	}
};

int main() {
	ImmutableText* insert = new ImmutableText(L"1");
	for (int j = 0; j < 100; j++) {
		ImmutableText* myText = new ImmutableText(L"hello");
		for (int i = 0; i < 1000; i++) {
			myText = myText->InsertText(i, insert);
		}
		for (int i = 0; i < 1000; i++) {
			myText = myText->RemoveText(0, 1);
		}
	}

	return 0;
}

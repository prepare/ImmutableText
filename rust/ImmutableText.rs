use std::cmp;

// <summary>Holds the default size for primitive blocks of characters.</summary>
const BLOCK_SIZE : i32 = 1 << 6;

/// <summary>Holds the mask used to ensure a block boundary cesures.</summary>
const BLOCK_MASK : i32 = !(BLOCK_SIZE - 1);

trait Node {
	fn length(&self) -> usize;
	fn sub_node(&self, start: usize, end: usize) -> Node;
	fn get_char_at(&self, offset : usize) -> char;
	fn copy_to(&self, source_index : usize, destination : &mut [char], destination_index : usize, count : usize);
}

struct WideLeafNode {
	data:Vec<char>
}

impl Node for WideLeafNode {
	fn length(&self) -> usize {
		self.data.len()
	}
	
	fn sub_node<'a>(&self, start: usize, end: usize) -> Node {
		if start == 0 && end == self.length() {
			return self;
		}
		let mut vec = Vec::with_capacity(end - start);
		let mut i = 0;
		for j in start..end {
			vec[i] = self.data[j];
			i = i + 1;
		}
		let mut result = WideLeafNode {
			data : vec
		};

		result
	}
	
	fn get_char_at(&self, offset : usize) -> char {
		self.data[offset]
	}

	fn copy_to(&self, source_index : usize, destination : &mut [char], destination_index : usize, count : usize) {
		for i in 0..count {
			destination[destination_index + i] = self.data[source_index +i];
		}
	}
}



/*
func (this WideLeafNode) CopyTo(sourceIndex int, destination []rune, destinationIndex int, count int) {
	copy(destination[destinationIndex:], this.data[sourceIndex:count])
}

type CompositeNode struct {
	count int
	head Node
	tail Node
}

func (this CompositeNode) Length() int {
	return this.count
}

func (this CompositeNode) SubNode(start int, end int) Node {
	var cesure = this.head.Length()
	if end <= cesure {
		return this.head.SubNode (start, end)
	}
	if start >= cesure {
		return this.tail.SubNode (start - cesure, end - cesure)
	}
	if start == 0 && end == this.count {
		return this
	}
	// Overlaps head and tail.
	return ConcatNodes (this.head.SubNode (start, cesure), this.tail.SubNode (0, end - cesure))
}

func (this CompositeNode) GetCharAt(index int) rune {
	var headLength = this.head.Length()
	if index < headLength {
		return this.head.GetCharAt(index)
	}
	return this.tail.GetCharAt(index - headLength)
}

func (this CompositeNode) CopyTo(sourceIndex int, destination []rune, destinationIndex int, count int) {
	var cesure = this.head.Length ()
	if sourceIndex + count <= cesure {
		this.head.CopyTo (sourceIndex, destination, destinationIndex, count)
		return
	}
	if (sourceIndex >= cesure) {
		this.tail.CopyTo (sourceIndex - cesure, destination, destinationIndex, count)
		return
	}
	// Overlaps head and tail.
	var headChunkSize = cesure - sourceIndex;
	this.head.CopyTo (sourceIndex, destination, destinationIndex, headChunkSize)
	this.tail.CopyTo (0, destination, destinationIndex + headChunkSize, count - headChunkSize)
}

func (this CompositeNode) RotateRight () CompositeNode {
	// See: http://en.wikipedia.org/wiki/Tree_rotation
	var P, isType = this.head.(CompositeNode);
	if !isType {
		return this // Head not a composite, cannot rotate.
	}
	var A = P.head
	var B = P.tail
	var C = this.tail
	var tailLength = B.Length() + C.Length ()
	return CompositeNode { A.Length () + tailLength, A, CompositeNode { tailLength, B, C } }
}

func (this CompositeNode) RotateLeft () CompositeNode {
	// See: http://en.wikipedia.org/wiki/Tree_rotation
	var Q, isType = this.tail.(CompositeNode)
	if !isType {
		return this // Tail not a composite, cannot rotate.
	}
	var B = Q.head
	var C = Q.tail
	var A = this.head
	var headLength = A.Length() + B.Length()
	return CompositeNode { headLength +  C.Length(), CompositeNode { headLength, A, B }, C }
}

func NodeOf (node Node, offset int, length int) Node {
	if (length <= BLOCK_SIZE) {
		return node.SubNode (offset, offset + length);
	}
	// Splits on a block boundary.
	var half = ((length + BLOCK_SIZE) >> 1) & BLOCK_MASK
	var head = NodeOf (node, offset, half)
	var tail = NodeOf (node, offset + half, length - half)
	return CompositeNode { head.Length() + tail.Length(), head, tail }
}

func ConcatNodes (node1 Node, node2 Node) Node  {
	// All Text instances are maintained balanced:
	//   (head < tail * 2) & (tail < head * 2)
	var length = node1.Length() + node2.Length()
	if length <= BLOCK_SIZE { // Merges to primitive.
		var mergedArray = make ([]rune, length)
		node1.CopyTo (0, mergedArray, 0, node1.Length())
		node2.CopyTo (0, mergedArray, node1.Length(), node2.Length())
		return CreateLeafNode (mergedArray)
	}
	// Returns a composite.
	var head = node1
	var tail = node2
	var compositeTail, isType = tail.(CompositeNode)
	if (head.Length() << 1) < tail.Length() && isType {
		// head too small, returns (head + tail/2) + (tail/2)
		if compositeTail.head.Length() > compositeTail.tail.Length() {
			// Rotates to concatenate with smaller part.
			compositeTail = compositeTail.RotateRight ()
		}
		head = ConcatNodes (head, compositeTail.head)
		tail = compositeTail.tail
	} else {
		var compositeHead, isType2 = head.(CompositeNode)
		if (tail.Length() << 1) < head.Length() && isType2 {
			// tail too small, returns (head/2) + (head/2 concat tail)
			if (compositeHead.tail.Length() > compositeHead.head.Length()) {
				// Rotates to concatenate with smaller part.
				compositeHead = compositeHead.RotateLeft ()
			}
			tail = ConcatNodes (compositeHead.tail, tail)
			head = compositeHead.head
		}
	}
	return CompositeNode { head.Length() + tail.Length(), head, tail }
}

func CreateLeafNode (str []rune) Node {
//			byte [] bytes = ToBytesIfPossible (str);
//			if (bytes != null)
//				return new Leaf8BitNode (bytes);
	return WideLeafNode { str}
}

type ImmutableText struct {
	root Node
}

func (this ImmutableText) Length() int {
	return this.root.Length()
}

func (this ImmutableText) GetCharAt(index int) rune {
	var leaf = this.FindLeaf(index, 0)
	return (*leaf.leafNode).GetCharAt(index - leaf.offset)
}

type InnerLeaf struct {
	leafNode *Node
	offset int
}

func (this ImmutableText) EnsureChunked() ImmutableText {
	var len = this.Length()
	var composite, isComposite = this.root.(CompositeNode)
	if len > BLOCK_SIZE && !isComposite {
		return ImmutableText { NodeOf (composite, 0, len) }
	}
	return this
}

func (this ImmutableText) FindLeaf(index int, offset int) InnerLeaf {
	var node = this.root
	for {
		if index >= node.Length() {
			return InnerLeaf { nil, -1 }
		}

		var composite, isComposite = node.(CompositeNode)
		if isComposite {
			if (index < composite.head.Length()) {
				node = composite.head
			} else {
				offset += composite.head.Length()
				index -= composite.head.Length()
				node = composite.tail
			}
			continue
		}

		return InnerLeaf { &node, offset }
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
func (this ImmutableText) Concat(that ImmutableText) ImmutableText {
	if that.Length() == 0 {
		return this
	}
	if this.Length() == 0 {
		return that
	}
	return ImmutableText { ConcatNodes (this.EnsureChunked().root, that.EnsureChunked().root) }
}

/// <summary>
/// Returns a portion of this text.
// </summary>
/// <returns>the sub-text starting at the specified start position and ending just before the specified end position.</returns>
func (this ImmutableText) GetText(start int, count int) ImmutableText {
	var end = start + count
//	if ((start < 0) || (start > end) || (end > Length)) {
//		throw new IndexOutOfRangeException (" start :" + start + " end :" + end + " needs to be between 0 <= " + Length)
//	}
	if start == 0 && end == this.Length() {
		return this
	}
	if start == end {
		return ImmutableText { WideLeafNode {make([]rune, 0)}}
	}
	return ImmutableText { this.root.SubNode (start, end) }
}

func (this ImmutableText) InsertText(index int, text ImmutableText) ImmutableText {
	return this.GetText (0, index).Concat (text).Concat (this.SubText (index))
}

func (this ImmutableText) InsertString(index int, text string) ImmutableText {
	return this.InsertText (index, CreateImmutableText (text))
}

func CreateImmutableText (text string) ImmutableText {
	return ImmutableText { WideLeafNode { []rune(text) } }
}

/// <summary>
/// Returns the text without the characters between the specified indexes.
/// </summary>
/// <returns><code>subtext(0, start).concat(subtext(end))</code></returns>
func (this ImmutableText) RemoveText(start int, count int) ImmutableText {
	if count == 0 {
		return this
	}
	var end = start + count
//	if (end > Length)
//		throw new IndexOutOfRangeException ();
	return this.EnsureChunked ().GetText (0, start).Concat (this.SubText (end))
}

func (this ImmutableText) SubText(start int) ImmutableText {
	return this.GetText (start, this.Length() - start)
}

func (this ImmutableText) ToString() string {
	var runes = make([]rune, this.Length())
	this.root.CopyTo(0, runes, 0, this.Length())
	return string(runes)
}
*/

fn main() {

  println!("Hello world");

  /*
	for j := 0; j < 100; j++ {
		var myText = CreateImmutableText("hello")
		for i := 0; i < 100000; i++ {
			myText = myText.InsertString(i, "1")
		}
		for i := 0; i < 100000; i++ {
			myText = myText.RemoveText(0, 1)
		}
	}
  */
}

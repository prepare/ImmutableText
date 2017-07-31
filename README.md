# ImmutableText data structure

A data structure allowing to store text and provide efficient insert and remove operations as well as O(1) snapshotting. Allowing to use the text on a background thread while continuing to modify it on the main thread. This is useful for text editors and IDEs.

# History.

This data structure was taken from intellij jetbrains and originiated in javolution.text.Text. It was adopted and used by monodevelop until recently.

# What this is good for
 This data structure is very efficient and I tried to start porting it to other languages. If you're in editor/ide development just pick it up and play around with it. It's way more advanced than gap buffers and faster than piece tables. There is no perfect data structure for that problem but if you need O(1) snapshotting that's the way to go.

# Comparison

There is a little speed test with inserts/removed in each implementation - currently I've following results:

go: 53.853s
csharp: 12.457s
java: 5.678s
c++: approx. ~38s - this implementation needs proper memory management
rust: todo

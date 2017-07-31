using System;

namespace ImmutableText
{
    class MainClass
    {
        public static void Main(string[] args)
        {
            for (int j = 0; j < 1000; j++)
            {
                var myText = new ImmutableText("hello");
                for (int i = 0; i < 1000; i++)
                {
                    myText = myText.InsertText(i, "1");
                }

                for (int i = 0; i < 1000; i++)
                {
                    myText = myText.RemoveText(0, 1);
                }
            }

        }
    }
}

#include "bits.h"
#include "treenode.h"
#include "huffman.h"
#include "map.h"
#include "vector.h"
#include "priorityqueue.h"
#include "strlib.h"
#include "testing/SimpleTest.h"
using namespace std;

string decodeText(EncodingTreeNode* tree, Queue<Bit>& messageBits) {

    string message = "";
    EncodingTreeNode* copyTree = tree;

    while(!messageBits.isEmpty())
    {
        Bit bitMove = messageBits.dequeue();
        if(bitMove == 0)
        {
            copyTree = copyTree->zero;
            if (copyTree->isLeaf())
            {
                message+= copyTree->ch;
                copyTree = tree;
            }
        }
        else
        {
            copyTree = copyTree->one;
            if (copyTree->isLeaf())
            {
                message+= copyTree->ch;
                copyTree = tree;
            }
        }
    }

    return message;
}

EncodingTreeNode* unflattenTree(Queue<Bit>& treeShape, Queue<char>& treeLeaves) {

        if(treeShape.peek() == 0)
        {
            char leafChar = treeLeaves.dequeue();
            treeShape.dequeue();
            EncodingTreeNode* leaf = new EncodingTreeNode(leafChar);
            return leaf;
        }

        treeShape.dequeue();
        EncodingTreeNode* inner = new EncodingTreeNode(nullptr, nullptr);
        inner-> zero = unflattenTree(treeShape, treeLeaves);
        inner-> one = unflattenTree(treeShape, treeLeaves);


     return inner;

}

string decompress(EncodedData& data) {

    EncodingTreeNode* tree = unflattenTree(data.treeShape, data.treeLeaves);
    string decompressed = decodeText(tree, data.messageBits);
    deallocateTree(tree);
    return decompressed;

}

Map<char, int> frequencyMap(string text)
{
    Map<char, int> freqMap;

    for(int i = 0; i < text.length(); i++)
    {
        if(!freqMap.containsKey(text[i]))
        {
            freqMap[text[i]] = 0;
        }
        freqMap[text[i]] += 1;
    }

    return freqMap;
}


EncodingTreeNode* buildHuffmanTree(string text) {

    PriorityQueue<EncodingTreeNode*> pq;
    Map<char, int> freqMap = frequencyMap(text);
    for(char key : freqMap.keys())
    {
        pq.enqueue(new EncodingTreeNode(key), freqMap[key]);
    }

    while (pq.size() > 1)
    {
        double pri1 = pq.peekPriority();
        EncodingTreeNode* zero = pq.dequeue();
        double pri2 = pq.peekPriority();
        EncodingTreeNode* one = pq.dequeue();
        EncodingTreeNode* merge = new EncodingTreeNode(zero, one);
        pq.enqueue(merge, pri1 + pri2);
    }

    EncodingTreeNode* finalTree = pq.dequeue();
    return finalTree;
}


Map<char, Queue<Bit>> encodeTextHelper(EncodingTreeNode* tree, Queue<Bit> seq) {

    Map<char, Queue<Bit>> charEncoded;

    if(tree->isLeaf())
    {
        charEncoded[tree->getChar()] = seq;
        return charEncoded;
    }

    Queue<Bit> seq0 = seq;
    seq0.enqueue(0);

    Queue<Bit> seq1 = seq;
    seq1.enqueue(1);

    charEncoded = encodeTextHelper(tree->zero, seq0) + encodeTextHelper(tree->one, seq1);
    return charEncoded;

}

Queue<Bit> encodeText(EncodingTreeNode* tree, string text) {

    Queue<Bit> finalSequence;
    Queue<Bit> seq;
    Map<char, Queue<Bit>> charEncoded = encodeTextHelper(tree, seq);

    for (int i = 0; i < text.length(); i++)
    {
        int letterMapSize = charEncoded[text[i]].size();
        Queue<Bit> temp = charEncoded[text[i]];
        for (int j = 0; j < letterMapSize; j++)
        {
                finalSequence.enqueue(temp.dequeue());
        }
    }

    return finalSequence;
}

void flattenTree(EncodingTreeNode* tree, Queue<Bit>& treeShape, Queue<char>& treeLeaves) {

    if (tree->isLeaf())
    {
            treeShape.enqueue(0);
            treeLeaves.enqueue(tree->getChar());
            return;
    }

    treeShape.enqueue(1);
    flattenTree(tree->zero, treeShape, treeLeaves);
    flattenTree(tree->one, treeShape, treeLeaves);
}

EncodedData compress(string messageText) {
    bool valid = false;
    char firstChar = messageText[0];
    for(char s: messageText)
    {
        if (s != firstChar)
        {
            valid = true;
        }
    }
    if(!valid)
    {
        error("Input requires atleast two distinct characters");
    }

    EncodingTreeNode* huff = buildHuffmanTree(messageText);
    Queue<Bit> treeShape;
    Queue<char> treeLeaves;
    flattenTree(huff, treeShape, treeLeaves);
    Queue<Bit> messageBits = encodeText(huff, messageText);
    EncodedData data;
    data.treeShape = treeShape;
    data.treeLeaves = treeLeaves;
    data.messageBits = messageBits;

    deallocateTree(huff);
    return data;
}


EncodingTreeNode* createExampleTree() {
    /* Example encoding tree used in multiple test cases:
     *                *
     *              /   \
     *             T     *
     *                  / \
     *                 *   E
     *                / \
     *               R   S
     */

    EncodingTreeNode* T = new EncodingTreeNode('T');
    EncodingTreeNode* R = new EncodingTreeNode('R');
    EncodingTreeNode* S = new EncodingTreeNode('S');
    EncodingTreeNode* E = new EncodingTreeNode('E');


    EncodingTreeNode* RS = new EncodingTreeNode(R, S);
    EncodingTreeNode* RSE = new EncodingTreeNode(RS, E);
    EncodingTreeNode* root = new EncodingTreeNode(T, RSE);


    return root;
}

void deallocateTree(EncodingTreeNode* t) {
    if (t == nullptr)
    {
        return;
    }
    deallocateTree(t->zero);
    deallocateTree(t->one);
    delete t;


}

bool areEqual(EncodingTreeNode* a, EncodingTreeNode* b) {

    if(a == nullptr && b == nullptr)
    {
        return true;
    }
    else if((a == nullptr && b != nullptr)  || (b == nullptr && a != nullptr))
    {
        return false;
    }
    else if( (a->isLeaf() && !b->isLeaf()) || (b->isLeaf() && !a->isLeaf()))
    {
        return false;
    }
    else if(a->isLeaf() && b->isLeaf())
    {
        if(a->ch == b->ch)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    return areEqual(a->zero, b->zero) && areEqual(a->one, b->one);

}



TEST("Extra Tests for decodeText")
{
    EncodingTreeNode* tree = createExampleTree();
    EXPECT(tree != nullptr);


    Queue<Bit>messageBits = {0};
    EXPECT_EQUAL(decodeText(tree, messageBits), "T");

    messageBits = {1,0,0,1,1,1,0,1};
    EXPECT_EQUAL(decodeText(tree, messageBits), "RES");
    deallocateTree(tree);

}



TEST("unflattenTree, further testing") {
    Queue<Bit>  treeShape  = { 1,0,1,1,0,0,0 };
    Queue<char> treeLeaves = { 'A','D','B','N' };
    EncodingTreeNode* tree = unflattenTree(treeShape, treeLeaves);

    deallocateTree(tree);
}



TEST("decompress, small example input") {
    EncodedData data = {
        { 1, 0 , 1,0,0 }, // treeShape
        { 'E','W','K' },  // treeLeaves
        { 1,1,0,1,0 } // messageBits
    };

    EXPECT_EQUAL(decompress(data), "KEW");

     data = {
        {1,0,1,1,0,0,0 },
        { 'A','D','B','N' },
        { 1,0,0,0,1,0,1}
    };

     EXPECT_EQUAL(decompress(data), "DAB");

     data = {
        {1,1,0,1,0,0,0 },
        { 'N','M','S','O' },
        {0,0,1,0,1,0,0,1,1}
    };

     EXPECT_EQUAL(decompress(data), "NOMS");
}


TEST("buildHuffmanTree, more examples")
{

     EncodingTreeNode* huff = buildHuffmanTree("BOOK");

     EncodingTreeNode* B = new EncodingTreeNode('B');
     EncodingTreeNode* K = new EncodingTreeNode('K');
     EncodingTreeNode* O = new EncodingTreeNode('O');

        EncodingTreeNode* KB = new EncodingTreeNode(K, B);
        EncodingTreeNode* KBO = new EncodingTreeNode(KB,O);

     EXPECT(areEqual(huff, KBO));

     deallocateTree(KBO);
     deallocateTree(huff);

}




TEST("encodeText testing") {
    EncodingTreeNode* tree = createExampleTree();

    Queue<Bit>messageBits = {0};
    EXPECT_EQUAL(encodeText(tree, "T"), messageBits);

    messageBits = {1,0,0,1,1,1,0,1};
    EXPECT_EQUAL(encodeText(tree, "RES"), messageBits);
    deallocateTree(tree);
}

TEST("flattenTree, more examples") {
    EncodingTreeNode* reference = createExampleTree(); // see diagram above

    Queue<Bit>  treeShape;
    Queue<char> treeLeaves;

    flattenTree(reference, treeShape, treeLeaves);
    EncodingTreeNode* final = unflattenTree(treeShape, treeLeaves);
    EXPECT(areEqual(reference, final));

    deallocateTree(reference);
    deallocateTree(final);

    Queue<Bit>  treeShape2  = { 1,0,1,1,0,0,0 };
    Queue<char> treeLeaves2 = { 'A','D','B','N' };
    EncodingTreeNode* tree = unflattenTree(treeShape2, treeLeaves2);
    treeShape.clear();
    treeLeaves.clear();
    flattenTree(tree, treeShape, treeLeaves);
    treeShape2  = { 1,0,1,1,0,0,0 };
    treeLeaves2 = { 'A','D','B','N' };
    EXPECT_EQUAL(treeShape,treeShape2);
    EXPECT_EQUAL(treeLeaves,treeLeaves2);

    deallocateTree(tree);
}

TEST("Testing Frequency Map Helper fucntion"){
    string text = "Mississippi";
    Map<char, int> freqMap = frequencyMap(text);
}



TEST("compress, more example testing") {
    EncodedData data = compress("BOOKKEEPER");
    string decoded = decompress(data);
    EXPECT_EQUAL(decoded,"BOOKKEEPER");

    EncodedData data1 = compress("Mississippi");
    string decoded1 = decompress(data1);
    EXPECT_EQUAL(decoded1,"Mississippi");

    EncodedData data2 = compress("BRAIN");
    string decoded2 = decompress(data2);
    EXPECT_EQUAL(decoded2,"BRAIN");

}


TEST("Call to Create and Deallocate to validate no memory leaks") {
    EncodingTreeNode* tree = createExampleTree();
    deallocateTree(tree);
}


TEST("Verfying areEqual function is wokring")
{
    EncodingTreeNode* single = new EncodingTreeNode('A');
    EncodingTreeNode* empty = nullptr;
    EXPECT_EQUAL(areEqual(single,empty), false);

    EncodingTreeNode* single2 = new EncodingTreeNode('A');
    EncodingTreeNode* single3 = new EncodingTreeNode('B');
    EXPECT_EQUAL(areEqual(single,single2), true);
    EXPECT_EQUAL(areEqual(single,single3), false);

    EncodingTreeNode* tree = createExampleTree();
    EXPECT_EQUAL(areEqual(tree,single), false);

    EncodingTreeNode* tree2 = createExampleTree();
    EXPECT_EQUAL(areEqual(tree,tree2), true);

    EncodingTreeNode* R = new EncodingTreeNode('R');
    EncodingTreeNode* S = new EncodingTreeNode('S');
    EncodingTreeNode* E = new EncodingTreeNode('E');
    EncodingTreeNode* RS = new EncodingTreeNode(R, S);
    EncodingTreeNode* RSE = new EncodingTreeNode(RS, E);
    EXPECT_EQUAL(areEqual(tree,RSE), false);

    deallocateTree(single);
    deallocateTree(empty);
    deallocateTree(single2);
    deallocateTree(single3);
    deallocateTree(tree);
    deallocateTree(tree2);
    deallocateTree(RSE);

}


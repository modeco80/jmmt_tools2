import std.mem;

// octree node
struct aOctree {
// bbox min
    float minX;
    float minY;
    float minZ;

    s16 patch; // First patch in this tree node
    s16 count; // How many other patches are included

// bbox max
    float maxX;
    float maxY;
    float maxZ;

    s16 sibling; // index in "array" to next
                 // node part of this node
    s16 child; // index in array to next node
                // that is anew in the hierarchy
};


aOctree octree[while(!std::mem::eof())] @ 0;

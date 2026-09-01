struct aVifPatch {
    u32 tag0;
    s32 rx;
    s32 ry;
    s32 rz;
    s32 rw;

    u32 tag1;
    u32 tag2;
    u16 xyz0[12];
    u16 xyz1[12];
    u16 xyz2[12];
    u16 xyz3[12];

    u32 tag3;
    u32 tag4;
    u16 rgb0[12];
    u16 rgb1[12];
    u16 rgb2[12];
    u16 rgb3[12];

    u32 tag5;
    u16 uv[10];
    u32 tag6;
};

struct aPatchHeader {
    float scale;
    u32 _res0;
    u32 _res1;
    u32 _res2;
    u32 patchCount;
    u32 patch;

    u32 sectorCount;
    u32 sector;

    u16 _res3[14];

    aVifPatch patches[2] @ patch;
};

aPatchHeader patchHeader @0;

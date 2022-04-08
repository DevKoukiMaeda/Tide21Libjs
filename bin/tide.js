(function () {
    Tide = {}
    TideMod().then(function(TModule) {
        Tide._free = TModule._free;
        Tide.allocate = TModule.allocate;
        Tide.ALLOC_NORMAL = TModule.ALLOC_NORMAL;
        Tide.intArrayFromString = TModule.intArrayFromString;
        Tide.UTF8ToString = TModule.UTF8ToString;
        Tide.MAXBUFFER = 1024*1024;
        Tide._calctide = TModule.cwrap(
            'calctide',
            null,
            ['number','number', 'number','number', 'number']);
        Tide._crtide =  TModule.cwrap(
            "cr_tide",
            null,
            ['number', 'number','number','number','number','number']);
          console.log("TideReady");
        Tide.calctide = function (y, m, d, td2path) {
          var ptr  = Tide.allocate(Tide.intArrayFromString(td2path),  Tide.ALLOC_NORMAL);
          var ptrRet = Tide.allocate(Array(Tide.MAXBUFFER).fill(0),Tide.ALLOC_NORMAL);
          Tide._calctide(y,m,d,ptr,ptrRet);
          var resstr = Tide.UTF8ToString(ptrRet);
          Tide._free(ptr);
          Tide._free(ptrRet);
          return JSON.parse(resstr);
        }
        Tide.crtide = function (y, m, d, h, mn, td2path) {
          var ptr  = Tide.allocate(Tide.intArrayFromString(td2path),  Tide.ALLOC_NORMAL);
          var ptrRet = Tide.allocate(Array(Tide.MAXBUFFER).fill(0),Tide.ALLOC_NORMAL);
          Tide._crtide(y, m, d, h, mn,ptr,ptrRet);
          var resstr = Tide.UTF8ToString(ptrRet);
          Tide._free(ptr);
          Tide._free(ptrRet);
          return JSON.parse(resstr);
        }
      });
}());
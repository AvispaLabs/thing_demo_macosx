function loop(params, context, done) {
    console.log("Running loop extension...");
    var dweetKey;
    if(params.objectScope.thingID == "th.92803ea00022-439a-5e11-3450-06389390")
      kiiVendorthingid = "thingid_772";
    if(params.objectScope.thingID == "th.d732c1a00022-272a-6e11-9300-0024927d")
      kiiVendorthingid = "thingid_3470";
    if(!kiiVendorthingid) {
      console.log("Error: Can't map thingId to vendorThingId");
      done("Error: Can't map thingId to vendorThingId");
    } else {
      var admin = context.getAppAdminContext();
      var bucket = admin.bucketWithName(kiiVendorthingid);
      var sensorEntry = admin.objectWithURI(params.uri);
      var newSensorEntry = bucket.createObject();
      refreshObject(sensorEntry, function(object) {
          newSensorEntry._customInfo = object._customInfo;
          saveObject(newSensorEntry, function(newObject) {
              createDweetFromKiiObject(kiiVendorthingid, newObject, dweetKey, null, done);
          }, done);
      }, done);
    }
}

function refreshObject(kiiObject, workerCallback, doneCallback) {
    kiiObject.refresh({
      success: function(theObject) {
        if(workerCallback) workerCallback(theObject);
        else if(doneCallback) doneCallback("SUCCESS!");
      },
      failure: function(theObject, errorString) {
        if(doneCallback) doneCallback(errorString);
      }
    });
}

function saveObject(kiiObject, workerCallback, doneCallback) {
    kiiObject.save({
      success: function(theObject) {
        if(workerCallback) workerCallback(theObject);
        else if(doneCallback) doneCallback("SUCCESS!");
      },
      failure: function(theObject, errorString) {
        if(doneCallback) doneCallback(errorString);
      }
    });
}

function loadThing(vendorThingId, context, workerCallback, doneCallback) {
    var admin = context.getAppAdminContext();
    admin.loadThingWithVendorThingID(vendorThingId, {
        success: function(thing) {
            console.log("Thing loaded!");
            if(workerCallback) workerCallback(thing);
            else if(doneCallback) doneCallback("SUCCESS!");
        },
        failure: function(error) {
            console.log("Can't load Thing: " + error);
            if(doneCallback) doneCallback(error);
        }
    });
}

function storeKiiThingObject(context, params, payload, workerCallback, doneCallback) {
    var kiiVendorthingid = params.elogStationid + params.elogServerid;
    $.ajax({
        url: 'https://api.kii.com/api/apps/' + context.getAppID() + '/things/VENDOR_THING_ID:' + kiiVendorthingid + '/buckets/' + params.kiiBucketName + '/objects',
        headers: {
            "X-Kii-AppID": context.getAppID(),
            "X-Kii-AppKey": context.getAppKey(),
            "Content-Type": "application/vnd.e951c9a1." + params.kiiBucketName + "+json",
            "Accept": "*/*",
            "Authorization": "Bearer " + params.kiiThingtoken
        },
        type: "POST",
        data: payload, // as json string
        json: "false",
        dataType: "json",
        success: function(body) {
            console.log("OK storing Kii object!");
            if(workerCallback) workerCallback(body);
            else if(doneCallback) doneCallback("SUCCESS!");
        },
        error: function(msg) {
            console.log("Error storing Kii object:" + JSON.stringify(msg));
            if(doneCallback) doneCallback(msg);
        }
    });
}

function getNewestThingSensorTimestamp(thing, bucketName, timestampKey, workerCallback, doneCallback) {
  var bucket = thing.bucketWithName(bucketName);
  // Build the query with the clauses and some other parameters
  var query = KiiQuery.queryWithClause();
  query.setLimit(1);
  query.sortByDesc(timestampKey);

  // Define the callbacks
  var queryCallbacks = {
    success: function(queryPerformed, resultSet, nextQuery) {
        if(resultSet.length > 0) {
            var obj = resultSet[0];
            if(workerCallback) workerCallback(obj.get(timestampKey));
            else if(doneCallback) doneCallback("SUCCESS!");
        }
        else {
            if(workerCallback) workerCallback(null);
            else if(doneCallback) doneCallback();
        }
    },
    failure: function(queryPerformed, anErrorString) {
      console.log("Error fetching sensor objects: " + anErrorString);
      if(doneCallback) doneCallback(anErrorString);
    }
  }
  // Execute the query
  bucket.executeQuery(query, queryCallbacks);
}

function fetchEnvirologgerData(params, length, workerCallback, doneCallback) {
    var elogSite = "http://api.envirologger.net/2.0/interface";
    var elogLatestAction = "stationdata/latest";

    $.ajax({
        url: 
            elogSite + "/" + 
            params.elogUserid + "/" + 
            params.elogLicencekey + "/" + 
            elogLatestAction + "/" + 
            params.elogServerid + "/" + 
            params.elogStationid + "/" + 
            params.elogDataCount,
        type: "GET",
        contentType:"application/json; charset=utf-8",
        dataType:"json",
        success: function(body) {
            if(workerCallback) workerCallback(body);
            else if(doneCallback) doneCallback("SUCCESS!");
        },
        error: function(msg) {
            console.log("Envirologger Error: " + msg);
            if(doneCallback) doneCallback(msg);
        }
    });
}

function createDweet(thingid, payload, key, workerCallback, doneCallback) {
  var dweetSite = "https://dweet.io:443";
  var dweetCreateAction = "/dweet/for/";
  var url = dweetSite + dweetCreateAction + thingid;
  if(key)
    url = url + "?key=" + key;
  console.log("Posting to " + url + "...");
  // Send visualization data
  $.ajax({
      url: url,
      type: "POST",
      data: payload, //as json string
      contentType:"application/json; charset=utf-8",
      dataType:"json",
      success: function(body) {
          console.log("Dweet sent!");
          if(workerCallback) workerCallback(body);
          else if(doneCallback) doneCallback("SUCCESS!");
      },
      error: function(msg) {
          console.log("Error Dweeting");
          if(doneCallback) doneCallback(msg);
      }
  });
}

function createDweetFromKiiObject(thingid, kiiObject, key, workerCallback, doneCallback) {
  var string = null;
  $.each(kiiObject, function(key, val){
      if(key == "_customInfo")
          string = JSON.stringify(val);
  });
  if (string == null){
      console.log("Error: can't retrieve thing custom data for dweeting");
      if(doneCallback) doneCallback("Dweet error");
  }
  else
    createDweet(thingid, string, key, workerCallback, doneCallback);
}

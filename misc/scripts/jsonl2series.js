const readline = require('readline');

const trailR = new RegExp(',}|,$');

// https://stackoverflow.com/questions/6156501/read-a-file-one-line-at-a-time-in-node-js
async function processLineByLine() {
  const rl = readline.createInterface({
    input: process.stdin,
    crlfDelay: Infinity
  });

  const s = {};
  for await (const line of rl) {
    const jl = line.replace(trailR, '}');
    try {
      const r = JSON.parse(jl);
      Object.entries(r).forEach((kv, i)=>{
        if(!(kv[0] in s)){
          s[kv[0]] = [];
        }
        if(typeof(kv[1])==='number'){
          s[kv[0]].push(kv[1]);
        }else{
          if(kv[1]){
            s[kv[0]].push(r.val);
          }else{
            s[kv[0]].push('');
          }
        }
      });
    } catch (e) {
      // console.log('failed');
    }
  }

  const series = {
    series: Object.entries(s).map((kv, i)=>({
        name: kv[0],
        data: kv[1],
    })).filter(v => ['millis', 'gyroX', 'gyroY', 'gyroZ', 'val','bpm', 'peakP'].indexOf(v.name) >= 0),
  };
  console.log(JSON.stringify(series));
}

processLineByLine();

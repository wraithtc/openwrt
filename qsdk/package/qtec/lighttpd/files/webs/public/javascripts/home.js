$('.router-content').fadeIn()
var downloadspeed=[];
var tempDownloadspeed=[];
var uploadspeed=[];
var tempUploadspeed=[];
var cpuload=[];
var ramload=[];
var time=[];
function chartinit() {
    for (var i = 0; i < 21; i++) {
        downloadspeed.push(0);
        tempDownloadspeed.push(0);
        uploadspeed.push(0);
        tempUploadspeed.push(0);
        cpuload.push(0);
        ramload.push(0);
        time.push(0);
    }
}
chartinit();
// 基于准备好的dom，初始化echarts实例
var downloadspeedChart = echarts.init(document.getElementById('downloadSpeedChart'));

// 指定图表的配置项和数据
var downloadspeedoption = {
    tooltip: {
        trigger: 'axis'
    },
    grid:{
        left: '15%',
        right: '0%',
        top:'5%',
        bottom: '3%',
    },
    xAxis:  {
        type: 'category',
        boundaryGap: false,
        data: time,
        axisTick:{
            show:false
        },
        axisLine:{
            lineStyle:{
                color:'#2196f3',
                width:1.5
            }
        }
    },
    yAxis: {
        type: 'value',
        min:0.00,
        max:150.00,
        boundaryGap: false,
        scale:true,
        splitNumber:5,
        axisLine:{
            show:false,
        },
        splitLine:{
            show:true,
        },
        axisLabel: {
            formatter: '{value}KB/s',
            textStyle:{
                fontFamily:'PingFangSC-Regular, PingFang SC,Microsoft YaHei,sans-serif',
                fontSize:14
            },
        },
    },
    series: [
        {
            type:'line',
            animation:false,
            data:downloadspeed,
            symbol:"none",
            animationEasing:'linear',
            animationEasingUpdata:'linear',
            areaStyle: {normal: {color:'#d3eafd'}},
            itemStyle : {
                normal : {
                    lineStyle:{
                        color:'#3ca3f4',
                        width:1.5
                    }
                }
            },
        },
    ]
};
function downloadSpeedChart() {
// 使用刚指定的配置项和数据显示图表。
    downloadspeedChart.setOption(downloadspeedoption);
}
var uploadspeedChart = echarts.init(document.getElementById('uploadSpeedChart'));
// 指定图表的配置项和数据
var uploadspeedoption = {
    tooltip: {
        trigger: 'axis'
    },
    grid:{
        left: '15%',
        right: '0%',
        top:'5%',
        bottom: '3%',
    },
    xAxis:  {
        type: 'category',
        boundaryGap: false,
        data: time,
        axisTick:{
            show:false
        },
        axisLine:{
            lineStyle:{
                color:'#2196f3',
                width:1.5
            }
        }
    },
    yAxis: {
        type: 'value',
        min:0.00,
        max:150.00,
        boundaryGap: false,
        scale:true,
        axisLine:{
            show:false,
        },
        splitLine:{
            show:true,
        },
        axisLabel: {
            formatter: '{value}KB/s',
            textStyle:{
                fontFamily:'PingFangSC-Regular, PingFang SC,Microsoft YaHei,sans-serif',
                fontSize:14
            },
        },
    },
    series: [
        {
            type:'line',
            animation:false,
            data:uploadspeed,
            symbol:"none",
            animationEasing:'linear',
            animationEasingUpdata:'linear',
            areaStyle: {normal: {color:'#d3eafd'}},
            itemStyle : {
                normal : {
                    lineStyle:{
                        color:'#3ca3f4',
                        width:1.5
                    }
                }
            },
        },
    ]
};
function uploadSpeedChart() {
// 使用刚指定的配置项和数据显示图表。
    uploadspeedChart.setOption(uploadspeedoption);
}

var cpuChart = echarts.init(document.getElementById('cpuChart'));
var cpuChartOption = {
    tooltip: {
        trigger: 'axis'
    },
    grid:{
        left: '15%',
        right: '0%',
        top:'5%',
        bottom: '3%',
    },
    xAxis:  {
        type: 'category',
        boundaryGap: false,
        data: time,
        axisTick:{
            show:false
        },
        axisLine:{
            lineStyle:{
                color:'#2196f3',
                width:1.5
            }
        }
    },
    yAxis: {
        type: 'value',
        min:0,
        max:100,
        boundaryGap: false,
        scale:true,
        axisLine:{
            show:false,
        },
        splitLine:{
            show:true,
        },
        axisLabel: {
            formatter: '{value}%',
            textStyle:{
                fontFamily:'PingFangSC-Regular, PingFang SC,Microsoft YaHei,sans-serif',
                fontSize:14
            },
        },
    },
    series: [
        {
            type:'line',
            animation:false,
            data:cpuload,
            symbol:"none",
            animationEasing:'linear',
            animationEasingUpdata:'linear',
            areaStyle: {normal: {color:'#d3eafd'}},
            itemStyle : {
                normal : {
                    lineStyle:{
                        color:'#3ca3f4',
                        width:1.5
                    }
                }
            },
        },
    ]
};
function cpuloadlist() {
    cpuChart.setOption(cpuChartOption);
}

var ramChart = echarts.init(document.getElementById('memoryChart'));
var ramChartOption = {
    tooltip: {
        trigger: 'axis'
    },
    grid:{
        left: '15%',
        right: '0%',
        top:'5%',
        bottom: '3%',
    },
    xAxis:  {
        type: 'category',
        boundaryGap: false,
        data: time,
        axisTick:{
            show:false
        },
        axisLine:{
            lineStyle:{
                color:'#2196f3',
                width:1.5
            }
        }
    },
    yAxis: {
        type: 'value',
        min:0,
        max:100,
        boundaryGap: false,
        scale:true,
        axisLine:{
            show:false,
        },
        splitLine:{
            show:true,
        },
        axisLabel: {
            formatter: '{value}%',
            textStyle:{
                fontFamily:'PingFangSC-Regular, PingFang SC,Microsoft YaHei,sans-serif',
                fontSize:14
            },
        },
    },
    series: [
        {
            type:'line',
            animation:false,
            data:ramload,
            symbol:"none",
            animationEasing:'linear',
            animationEasingUpdata:'linear',
            areaStyle: {normal: {color:'#d3eafd'}},
            itemStyle : {
                normal : {
                    lineStyle:{
                        color:'#3ca3f4',
                        width:1.5
                    }
                }
            },
        },
    ]
};
function ramloadlist() {
    ramChart.setOption(ramChartOption);
}

downloadSpeedChart();
uploadSpeedChart();
cpuloadlist();
ramloadlist();
loadInfo();
var loadInterval;
function loadInfo() {
    getinfo();
    getblackinfo();
    getrouterInfo();
    clearTimeout(loadInterval);
    loadInterval=setTimeout(function () {
        loadInfo()
    },3000);
}

function getblackinfo() {
    var data = {
        "requestUrl": "proc_macblock_cfg",
        "method": "get"
    }
    postAjax(data,black_list)
}
function black_list(res) {
    if (res.errorcode == 0) {
        var data = res.data;
        if (data.length == 0) {
            $('.black-list-content').html('<div class="black-device-none">' + '<span>' + '黑名单暂无设备' + '</span>' + '</div>');
        } else {
            var compiled = _.template(document.getElementById("blackListTemplate").innerHTML);
            var str = compiled(data);
            $('.black-list-content').html(str);
        }
    }else {
        err.error_msg(res);
    }
}
var listupdate=1,deviceData;
function getinfo() {
    var data={
        "requestUrl": "get_stalist_cfg",
        "method": "get"
    }
   postAjax(data,device_list)
}
function device_list(res) {
    if(res.errorcode==0)
    {
        $('.normal-name').html('2.4G: '+res.data.lfssid);
        $('.speed-name').html('5G:'+res.data.hfssid)
        var n=0,list=[];
        var stalist=res.data.stalist;
        $ownip=res.data.ownip;
        for(var i=0;i<stalist.length;){
            // for(var j=0;j<blackinfo.length;){
            //    if( blackinfo[j].macaddr==res.data.stalist[i].macaddr){
            //        res.data.stalist[i].stastatus=0;
            //        res.data.stalist.splice(i)
            //    }
            //     j++;
            // }
            // var n=res.data.stalist[i].tx;
            // var m=res.data.stalist[i].rx;
            var t=stalist[i].stastatus;
            if(t<60 && t>0) {
                res.data.stalist[i].time = 1 + '分'
            }
            if(t>=60 && t<3600){
                res.data.stalist[i].time=Math.floor(t/60)+'分';
            }
            if(t>=3600 && t<86400){
                res.data.stalist[i].time=Math.floor(t/3600)+'小时'+Math.floor((t-Math.floor(t/3600)*3600)/60)+'分';
            }
            if(t>=86400){
                var day=Math.floor(t/86400);
                var h=Math.floor((t-day*86400)/3600);
                var m=Math.floor((t-day*86400-h*3600)/60);
                res.data.stalist[i].time=day+'天'+h+'小时'+m+'分';
            }
            // res.data.stalist[i].tx=(n*0.001).toFixed(2);
            // res.data.stalist[i].rx=(m*0.001).toFixed(2);
            res.data.stalist[i].ownip=res.data.ownip;
            if(t==0){
                n++;
            }else {
                list.push(stalist[i]);
            }
            i++;
        }
        $('.connect-num').html(res.data.devnum-n+'台');
        if(listupdate==1){
            deviceData = list.sort(function (a,b) {
                return a.stastatus-b.stastatus;
            });
            var compiled = _.template(document.getElementById("deviceListTemplate").innerHTML);
            var str = compiled(deviceData);
            $('.device-list').html(str);
        }
    }else {
        err.error_msg(res);
    }
}
function getrouterInfo() {
    var data={
        "requestUrl": "get_routerstatus_cfg",
        "method": "get"
    }
   postAjax(data,router_info_fill)
}
function router_info_fill(res) {
    if(res.errorcode==0)
    {
        $('.model-info').html(res.data.routername);
        $('.rom-info').html(res.data.romVersion);
        $('.router-mac').html(res.data.macaddr);
        $('.sn-info').html(res.data.sn);
        var t=(new Date()).getTime()
        if(time.length<20){
            time.push(t);
        }else {
            time.shift();
            time.push(t);
        }
        var routerrx=(res.data.routerrx/1024).toFixed(2);
        tempDownloadspeed.shift();
        tempDownloadspeed.push(routerrx);
        var maxspeed=Math.max.apply(null,tempDownloadspeed)
        downloadspeed=tempDownloadspeed.concat();
        if(maxspeed<=1024){
            switch(true){
                case maxspeed<=150:
                    downloadspeedoption.yAxis.max=150.00;
                    break;
                case  maxspeed>150 && maxspeed<=250:
                    downloadspeedoption.yAxis.max=250.00;
                    break;
                case maxspeed>250 && maxspeed<=500:
                    downloadspeedoption.yAxis.max=500.00;
                    break;
                case maxspeed>500 && maxspeed<=1000:
                    downloadspeedoption.yAxis.max=1000.00;
                    break;
                default:
                    break;
            }
            downloadspeedoption.yAxis.axisLabel.formatter='{value}KB/s'
        }else {
            for(var i=0;i<downloadspeed.length;i++) {
                if(downloadspeed[i]>0){
                    downloadspeed[i] = (downloadspeed[i] / 1024).toFixed(2);
                }
            }
            maxspeed=maxspeed/1024;
            switch(true){
                case maxspeed<=10:
                    downloadspeedoption.yAxis.max=10.00;
                    break;
                case maxspeed>10 && maxspeed<=20:
                    downloadspeedoption.yAxis.max=20.00;
                    break;
                case maxspeed>20 && maxspeed<=50:
                    downloadspeedoption.yAxis.max=50.00;
                    break;
                case maxspeed>50 && maxspeed<=100:
                    downloadspeedoption.yAxis.max=100.00;
                    break;
                case maxspeed>100 && maxspeed<=200:
                    downloadspeedoption.yAxis.max=200.00;
                    break;
                case maxspeed>200 && maxspeed<=500:
                    downloadspeedoption.yAxis.max=500.00;
                    break;
                case maxspeed>500 && maxspeed<=1024:
                    downloadspeedoption.yAxis.max=1024.00;
                    break;
                case maxspeed>1024:
                    downloadspeedoption.yAxis.max=maxspeed;
                    break;
                default:
                    break;
            }
            downloadspeedoption.yAxis.axisLabel.formatter='{value}MB/s'
        }
        downloadspeedoption.series[0].data=downloadspeed;
        downloadSpeedChart();

        if(routerrx<=1024){
            var routerrxunit='KB/s';
        }else {
            routerrx=(routerrx/1024).toFixed(2)
            var routerrxunit='MB/s';
        }
        var hrx=(res.data.hrx/1024).toFixed(2)
        if(hrx<1024){
            var hrxunit='KB/s';
        }else {
            hrx=(hrx/1024).toFixed(2)
            var hrxunit='MB/s';
        }
        $('.fast-download').html(hrx+hrxunit);
        var htx=(res.data.htx/1024).toFixed(2)
        if(htx<1024){
            var htxunit='KB/s';
        }else {
            htx=(htx/1024).toFixed(2)
            var htxunit='MB/s';
        }
        $('.fast-upload').html(htx+htxunit);
        $('.cpu-load').html(res.data.cpuusage);
        $('.core-num').html(res.data.cpunum);
        $('.core-frequency').html(res.data.cpuHz);
        var used=res.data.ramuse;
        var all=res.data.ramsize;
        var ramloadpercent=(used/all*100).toFixed(2);
        $('.memory-overthead').html(ramloadpercent+'%');
        $('.memory-capacity').html((all/1024).toFixed(2)+'MB');
        $('.memory-type').html(res.data.ramtype);
        $('.memory-frequency').html(res.data.ramHz);
        $('.download-speed').html(routerrx+routerrxunit);
        if(routerrx>=128){
            routerrx=routerrx/1024;
            var routerrxbandunit='Mbps';
        }else {
            var routerrxbandunit='Kbps';
        }
        $('.download-bandwidth').html((routerrx*8).toFixed(2)+routerrxbandunit);
        $('.speed-num').html('网速'+(routerrx*8).toFixed(2)+routerrxbandunit);

        var routertx=(res.data.routertx/1024).toFixed(2)
        tempUploadspeed.shift();
        tempUploadspeed.push(routertx);
        var maxuploadspeed=Math.max.apply(null,tempUploadspeed);
        uploadspeed=tempUploadspeed.concat();
        if(maxuploadspeed<=1024){
            switch(true){
                case maxuploadspeed<=150:
                    uploadspeedoption.yAxis.max=150.00;
                    break;
                case  maxuploadspeed>150 && maxuploadspeed<=250:
                    uploadspeedoption.yAxis.max=250.00;
                    break;
                case maxuploadspeed>250 && maxuploadspeed<=500:
                    uploadspeedoption.yAxis.max=500.00;
                    break;
                case maxuploadspeed>500 && maxuploadspeed<=1000:
                    uploadspeedoption.yAxis.max=1000.00;
                    break;
                default:
                    break;
            }
            uploadspeedoption.yAxis.axisLabel.formatter='{value}KB/s'
        }else {
            for(var i=0;i<uploadspeed.length;i++){
                uploadspeed[i]=(uploadspeed[i]/1024).toFixed(2);
            }
            maxuploadspeed=maxuploadspeed/1024
            switch(true){
                case maxuploadspeed<=10:
                    uploadspeedoption.yAxis.max=10.00;
                    break;
                case maxuploadspeed>10 && maxuploadspeed<=20:
                    uploadspeedoption.yAxis.max=20.00;
                    break;
                case maxuploadspeed>10 && maxuploadspeed<=50:
                    uploadspeedoption.yAxis.max=50.00;
                    break;
                case maxuploadspeed>50 && maxuploadspeed<=100:
                    uploadspeedoption.yAxis.max=100.00;
                    break;
                case maxuploadspeed>100 && maxuploadspeed<=200:
                    uploadspeedoption.yAxis.max=200.00;
                    break;
                case maxspeed>200 && maxspeed<=500:
                    uploadspeedoption.yAxis.max=500.00;
                    break;
                case maxspeed>500 && maxspeed<=1000:
                    uploadspeedoption.yAxis.max=1000.00;
                    break;
                case maxspeed>1000:
                    uploadspeedoption.yAxis.max=maxspeed;
                    break;
                default:
                    break;
            }
            uploadspeedoption.yAxis.axisLabel.formatter='{value}MB/s'
        }
        uploadspeedoption.series[0].data=uploadspeed;
        uploadSpeedChart();
        if(routertx<1024){
            var routertxunit='KB/s';
        }else {
            routertx=(routertx/1024).toFixed(2)
            var routertxunit='MB/s';

        }

        $('.upload-speed').html(routertx+routertxunit);
        if(routertx>=128){
            routertx=routertx/1024
            var routertxbandunit='Mbps';
        }else {
            var routertxbandunit='Kbps';
        }
        $('.upload-bandwidth').html((routertx*8).toFixed(2)+routertxbandunit);


        cpuload.shift();
        cpuload.push(res.data.cpuusage.replace('%',''));
        cpuloadlist();
        ramload.shift();
        ramload.push(ramloadpercent);
        ramloadlist();
    }else {
        err.error_msg(res);
    }
}
var manualdownload,manualupload;
function getstaticInfo() {
    var data={
        "requestUrl": "get_wan_detial_cfg",
        "method": "get"
    }
   postAjax(data,wan_info_list)
    var data={
        "requestUrl": "proc_qos_cfg",
        "method": "get"
    }
    postAjax(data,qos_info_list)
}
function wan_info_list(res) {
    if(res.errorcode==0)
    {
        $('.conntect-way').html(res.data.connectiontype.toUpperCase());
        $('.extranet-ip').html(res.data.ipaddr);
        $('.gateway').html(res.data.gateway);
        $('.preferreddns').html(res.data.dns1);
        $('.sparedns').html(res.data.dns2);
    }
    else {
        err.error_msg(res);
    }
}
function qos_info_list(res) {
    if (res.errorcode == 0) {
        manualdownload=res.data.download;
        manualupload=res.data.upload;
        if(manualdownload>0){
            if(res.enabled==1){
                $('.download-boradband').html((manualdownload/1024).toFixed(2)+'Mbps');
            }else {
                // $('.download-boradband').html((manualdownload/1024).toFixed(2)+'Mbps'+"<a class='qos-link' href='./extend.html#qos'  target='_self'>(未生效,点击设置)</a>");
                $('.download-boradband').html((manualdownload/1024).toFixed(2)+'Mbps');
            }
        }else {
            $('.download-boradband').html('--Mbps');
        }
        $('.manual-setting-download').val((manualdownload/1024).toFixed(2));
        if(manualupload>0){
            if(res.enabled==1){
                $('.upload-boradband').html((manualupload/1024).toFixed(2)+'Mbps');
            }else {
                // $('.upload-boradband').html((manualupload/1024).toFixed(2)+'Mbps'+"<a class='qos-link' href='./extend.html#qos'  target='_self'>(未生效,点击设置)</a>");
                $('.upload-boradband').html((manualupload/1024).toFixed(2)+'Mbps');
            }
        }else {
            $('.upload-boradband').html('--Mbps');
        }
        $('.manual-setting-upload').val((manualupload/1024).toFixed(2));
    }else {
        err.error_msg(res);
    }
}
$('.logout').click(function () {
    window.localStorage.removeItem('qtec_router_token');
    location.href='./login.html?t='+Math.random();
})
// var getinfoInterval;
// $('.device-icon').click(function () {
//     clearTimeout(loadInterval);
//     getinfo();
//     getinfoInterval=setTimeout(function () {
//         getinfo()
//     },3000);
// })
// var getstaticInfoInterval;
// $('.router-icon').click(function () {
//     clearTimeout(getinfoInterval);
//     getstaticInfo();
//     setTimeout(function () {
//         getstaticInfo()
//     },3000);
// })


$('.internet-icon').click(function () {
    getstaticInfo();
})
$('.manage-choose>div').click(function(){
    var i=$(this).index();
    if($('.content>div').eq(i).hasClass('hide')){
        $('.content>div').addClass('hide');
        $('.content>div').removeAttr('style');
        $('.content>div').eq(i).fadeIn();
        $('.content>div').eq(i).removeClass('hide');
        $('.manage-choose>div.active').removeClass('active');
        $('.manage-choose>div').eq(i).addClass('active')
        if(i==0){
            $('.normal li').eq(0).attr('class','normal-header fl active');
            $('.list>div').eq(0).attr('class','device-list');
            $('.normal li').eq(1).attr('class','blacklist-header fr');
            $('.list>div').eq(1).attr('class','black-list-content hide');
        }
    }
})
$('.normal li').click(function () {
   if(!( $(this).hasClass('active'))){
       var i=$('.normal li.active').index();
       $('.normal li.active').removeClass('active');
       $(this).addClass('active');
       $('.list>div').eq(i).removeAttr('style');
       $('.list>div.hide').fadeIn();
       $('.list>div.hide').removeClass('hide')
       $('.list>div').eq(i).addClass('hide');
   }
})
$('.limit-info input').click(function () {
    $(this).is(":checked")? $('.limit-info label').text('是'):$('.limit-info label').text('否');
})
$(document).on('input','.limit-speed input',function () {
    $(this).val($(this).val().replace(/\D/g,''));
    if($(this).val()>1000){
        $(this).val(1000)
    }
})
var limitmac,olddownlimit,olduplimit,enabled;
function speedlimit(n,a,b,c) {
    if($('.limit-setting').eq(n).hasClass('hide')){
        limithide();
        listupdate=0;
        var obj=$('.speed-limit').eq(n);
        obj.attr('disabled','disabled');
        obj.addClass('disabled');
        obj.find('.disabled').removeClass('hide')
        var evt = evt || window.event; //获取event对象
        if (evt.preventDefault) {
            evt.preventDefault(); //非IE浏览器
        } else {
            evt.returnValue = false; //在早期的IE版本中
        }
        event.stopPropagation ? event.stopPropagation() : (event.cancelBubble = true); //阻止事件冒泡
        var data={
            "data": {
                "mac":b,
            },
            "requestUrl": "ebtables_proc_speedlimit_cfg",
            "method": "get"
        }
        // postAjax(data,device_limit)
        $.ajax({
            type:'POST',
            url:'/cgi-bin/web.cgi/',
            data:JSON.stringify(data),
            dataType:'json',
            success:function (res) {
                if(res.errorcode==0)
                {
                    limitmac=b;
                    olddownlimit=res.data.downlimit;
                    olduplimit=res.data.uplimit;
                    enabled=res.data.enabled;
                    $('.limit-speed-download').eq(n).val(olddownlimit/1024);
                    $('.limit-speed-upload').eq(n).val(olduplimit/1024);
                }else {
                    err.error_msg(res);
                }
                obj.removeAttr('disabled');
                obj.removeClass('disabled');
                obj.find('.disabled').addClass('hide')
                // $('.limit-speed').removeClass('hide');
                $('.limit-setting').eq(n).removeClass('hide');
                $('.limit-setting').eq(n).slideDown('normal');
            },
        })
    }else {
        limithide();
        // $('.limit-setting').eq(n).addClass('hide');
    }
}
// function device_limit(res) {
//
// }
$(document).on('click','.limit-cancel',function () {
    limithide();
})
function limithide() {
    $('.limit-setting').addClass('hide');
    listupdate=1;
}

$(document).click(function (e) {
    e = window.event || e; // 兼容IE7
    var obj = $(e.srcElement || e.target);
    if(!obj.is('.limit-setting,.limit-setting *')){
        limithide();
    }
});
$(document).on('focus','.limit-setting input',function () {
    if($(this).val()==0){
        $(this).val('');
    }
})
$(document).on('blur','.limit-setting input',function () {
    if($(this).val().length==0){
        $(this).val(0);
    }
})
function forbid(a,b,c,e) {
    // e.stopPropagation();
    if(c==$ownip){
        resultTipshow(0,'本机不能拉黑')
        return;
    }
    // if($('.black-device-tip').hasClass('hide')){
    //     $('.black-device-tip').removeClass('hide')
    //     $('.black-device-model-name').html(a);
    //     $('.black-device-ip').html(c);
    //     $('.black-device-mac').html(b);
    // }
    resultTipshow(-1,-1);
    var data={"data": {
        "name":a,
        "macaddr": b,
        "enabled": 1
    },
        "requestUrl": "proc_macblock_cfg",
        "method": "post"
    }
    postAjax(data,black_device)
}
function black_device(res) {
    if(res.errorcode==0)
    {
        getinfo();
        getblackinfo();
        // for(var i=0;i<deviceData.length;i++){
        //     if(deviceData[i].macaddr==b){
        //         deviceData.splice(i,1)
        //     }
        //     var compiled = _.template(document.getElementById("deviceListTemplate").innerHTML);
        //     var str = compiled(deviceData);
        //     $('.device-list').html(str);
        // }
        resultTipshow(1,'保存成功');
    }else{
        err.error_msg(res);
    }
    $('.black-device-tip').addClass('hide');
}
$('.black-cancel').click(function () {
    $('.black-device-tip').addClass('hide');
})
$('.cancel').click(function () {
    $(this).parent().parent().parent().parent().fadeOut();
    $('.popup-shadow').addClass('hide');
})
$('.limit-speed-content input').on('input',function () {
    $(this).val($(this).val().replace(/\D/g,''))
})
$(document).on('click','.limit-confirm',function () {
    resultTipshow(-1,-1);
    var newenabled;
    var newdownlimit=$(this).parent().parent().find('.limit-speed-download').val();
    var newuploadlimit=$(this).parent().parent().find('.limit-speed-upload').val();
    if(newdownlimit>0 || newuploadlimit>0){
        newenabled=Number(1);
    }else {
        newenabled=Number(0);
    }
    var data={
        "data":{
            "mac": limitmac,
            "olddownlimit":Number(olddownlimit),
            "olduplimit":Number(olduplimit),
            "oldenabled": Number(enabled),
            "newdownlimit":Number(newdownlimit*1024),
            "newuplimit":Number(newuploadlimit*1024),
            "newenabled":newenabled
        },
        "requestUrl": "ebtables_proc_speedlimit_cfg",
        "method": "put"
    }
    postAjax(data,speed_limit_set)
})
function speed_limit_set(res) {
    if(res.errorcode==0)
    {
        limithide()
        resultTipshow(1,'修改成功');
    }else{
        err.error_msg(res);
    }
}
function blackdel(m,n) {
    resultTipshow(-1,-1);
    var data={
        "data": {
            "name":m,
            "macaddr":n,
            "enabled": 1
        },
        "requestUrl": "proc_macblock_cfg",
        "method": "delete"
    }
   postAjax(data,black_device_del)
}
function black_device_del(res) {
    if(res.errorcode==0)
    {
        getinfo();
        getblackinfo();
        // $('.black-device-remove').addClass('hide');
        resultTipshow(1,'移除成功');
    }else{
        err.error_msg(res);
    }
}
$('.limit-speed-box').click(function (e) {
    e.stopPropagation();
})
$('.black-device-tip-box').click(function (e) {
    e.stopPropagation();
})
$('.limit-speed').click(function () {
    $(this).addClass('hide');
})
$('.black-device-tip').click(function () {
    $(this).addClass('hide');
})
$('.boradband-test').click(function () {
    removeShadow('.boradband-testing')
})
var speedTestingChart = echarts.init(document.getElementById('speedTestingCanvas'));
var option = {
    tooltip : {
        formatter: "{a} <br/>{b} : {c}%"
    },
    series: [
        {
            type: 'gauge',
            z: 3,
            detail: {formatter:'{value}'},
            data: [{value: 0,name: 'MB/s'}],
            max:100,
            axisLine: {            // 坐标轴线
                lineStyle: {       // 属性lineStyle控制线条样式
                    width: 10
                }
            },
            axisTick: {            // 坐标轴小标记
                length: 15,        // 属性length控制线长
                lineStyle: {       // 属性lineStyle控制线条样式
                    color: 'auto'
                }
            },
            splitLine: {           // 分隔线
                length: 20,         // 属性length控制线长
                lineStyle: {       // 属性lineStyle（详见lineStyle）控制线条样式
                    color: 'auto',
                    fontSize: 20,
                }
            },
            title : {
                textStyle: {       // 其余属性默认使用全局文本样式，详见TEXTSTYLE
                    fontWeight: 'normal',
                    fontSize: 20,
                    fontStyle: 'italic'
                }
            },
            detail : {
                textStyle: {       // 其余属性默认使用全局文本样式，详见TEXTSTYLE
                    fontWeight: 'normal',
                    fontSize: 18,
                    textAlign:'center'
                }
            },
            pointer: {
                width:7
            },
        }
    ]
};
speedTestingChart.setOption(option);
$('.speed-testing-confirm').click(function () {
    $('.boradband-testing').hide();
    speedteststart()
    // window.setTimeout(function () {
    //     option.series[0].data[0].value = 1000;
    //     speedTestingChart.setOption(option, true);
    // },500)
    // window.setTimeout(function () {
    //     option.series[0].data[0].value = 0;
    //     speedTestingChart.setOption(option, true);
    // },1300)
    // window.setTimeout(function () {
    //     speedtesting();
        // for(var c=0;c<11;c++){
        //     if(c<10){
        //         window.setTimeout(function () {
        //             option.series[0].data[0].value = (Math.random() * 100).toFixed(2) - 0;
        //             speedTestingChart.setOption(option, true);
        //         },c*1000)
        //     }else{
        //         window.setTimeout(function () {
        //             option.series[0].data[0].value = 0;
        //             speedTestingChart.setOption(option, true);
        //             window.setTimeout(function () {
        //                 $('.testing-cancel').parent().parent().parent().addClass('hide');
        //                 $('.speed-testing-results').removeClass('hide');
        //             },200)
        //         },c*1100)
        //     }
        // }
    // },300)
})
var speedTestingSwitch;
function speedteststart() {
    clearTimeout(loadInterval)
    $('.speed-testing').fadeIn()
    var data={
        "data":{
            "action":1
        },
        "requestUrl": "proc_wan_speedtest_cfg",
        "method": "post"
    }
    postAjax(data,speed_test_start)
}
function speed_test_start(res) {
    if(res.errorcode==0){
        speedtesting();
        speedTestingSwitch = 1;
    }else {
        addShadow('.speed-testing');
        err.error_msg(res);
    }
}
var speedTestingInterval;
function speedtesting() {
    var data={
        "requestUrl": "proc_wan_speedtest_cfg",
        "method": "get"
    }
    postAjax(data,speed_testing,speedtesting)
}
function speed_testing(res) {
    if(res.errorcode==0){
        // clearTimeout(speedTestingInterval)
        if(res.data.speedtest==0){
            var download=(res.data.downspeed/1048576).toFixed(2);
            option.series[0].data[0].value=download;
            speedTestingChart.setOption(option, true);
            $('.real-speed').html(download+'MB/s');
            speedTestingInterval=setTimeout(function () {
                if(speedTestingSwitch==1){
                    speedtesting();
                }
            },300);
        }
        if(res.data.speedtest==1){
            addHide('.speed-testing');
            removeShadow('.speed-testing-results');
            $('.real-speed').html('');
            option.series[0].data[0].value = 0;
            speedTestingChart.setOption(option, true);
            $('.testing-result').html(Number(res.data.download).toFixed(2)+'Mbps');
            // $('.testing-result-upload').html(Number(res.data.upload).toFixed(2)+'Mbps');
            loadInfo();
        }
    }else {
        err.error_msg(res);
    }
}
// function speedGet() {
//     var data={
//
//     }
//     var speed=$.ajax({
//         type:'POST',
//         url:'/cgi-bin/web.cgi/',
//         data:JSON.stringify(data),
//         datatype:'json',
//         success:function (res) {
//
//         }
//     })
// }

$('.testing-cancel').click(function () {
    addShadow('.speed-testing');
    speedTestingSwitch=0;
    var data={
        "data":{
            "action":0
        },
        "requestUrl": "proc_wan_speedtest_cfg",
        "method": "post"
    }
   postAjax(data,speed_test_cancel)
;    loadInfo();
})
function speed_test_cancel(res) {
    if(res.errorcode==0) {
        clearTimeout(speedTestingInterval)
    }else {
        err.error_msg(res);
    }
}
$('.boradband-setting').click(function () {
    // getstaticInfo();
    // removeShadow('.manual-setting');
    location.href='./extend.html#qos?t='+Math.v()
})
$('.manual-setting-content input').on('input',function () {
    var v=$(this).val();
})
$('.manual-setting-content input').on('input',function () {
    if($('.tip-info').html().length>0){
        $('.tip-info').removeClass('red')
        $('.tip-info').html('')
    }
})
function speedtip(e) {
    $('.tip-info').html(e);
    $('.tip-info').addClass('red');
    window.setTimeout(function () {
        $('.tip-info').removeClass('red');
        window.setTimeout(function () {
            $('.tip-info').addClass('red');
        },300)
    },250)
}
$('.manual-setting-cancel').click(function () {
    $('.manual-setting-download').val((manualdownload/1024).toFixed(2));
    $('.manual-setting-upload').val((manualupload/1024).toFixed(2));
})
$('.upload-unit').on('input',function () {
    var val=$(this).val()
    var l=(val.split('.')).length-1;
    if(l>1){
        errorShow('.upload-unit','格式错误');
    }else {
        errorhide();
    }
})
$('.manual-setting-confirm').click(function () {
    var download=$('.manual-setting-download').val();
    var upload=$('.manual-setting-upload').val();
    var reg=/(^[0-9]{1,}$)|(^[0-9]{1,}[\.]{1}[0-9]{1,2}$)/;
    if(!reg.test(download) || !reg.test(upload)){
        speedtip('请输入0-1000以内的数值,最多2位小数');
        return
    }
    if(download>1000 || upload>1000){
        speedtip('0代表不限速，最大值为1000');
        return
    }
    addHide('.manual-setting');
    resultTipshow(-1,-1);
    var data={
        "data": {
            "qosmode":-99,
            "enabled": -99,
            "download": Number(download*1024),
            "upload": Number(upload*1024),
        },
        "requestUrl": "proc_qos_cfg",
        "method": "put"
    }
    var self=this;
    $.ajax({
        type:'POST',
        url:'/cgi-bin/web.cgi/',
        data:JSON.stringify(data),
        datatype:'json',
        success:function (res) {
            if(res.errorcode==0){
                resultTipshow(1,'保存成功');
                getstaticInfo();
            }else {
                err.error_msg(res);
            }
        }
    })
})
function manualSetting(){
    $('.speed-testing-results').hide()
    $('.boradband-setting').click();
}
$('.restart-testing').click(function () {
    $('.speed-testing-results').hide();
    speedteststart()
})
$('.result-confirm').click(function () {
    addShadow('.speed-testing-results')
})

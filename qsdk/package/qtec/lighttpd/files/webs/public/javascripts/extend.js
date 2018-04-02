// window.onscroll=function () {
//     var w=document.body.clientWidth;
//     if(w>1600){
//         var n=window.pageYOffset;
//         $('.nav-list').attr('style','top:'+n+'px');
//     }
// }
resize();
window.onresize=function () {
    resize()
} ;
function resize() {
    $('.nav-list').removeClass('hide')
    var w=document.body.clientWidth;
    var nl=(w-1160)/2-222;
    if(w<1603){
        $('.nav-list').addClass('row-list');
        $('.nav-list').removeAttr('style');
        spacebetween();
        $('.container').css('padding-top','106px');
    }else {
        $('.nav-list').attr('style','left:'+nl+'px')
        $('.nav-list').removeClass('row-list');
        $('.nav-list li').removeAttr('style');
        $('.container').removeAttr('style');
    }
}
var antiIntervalenable;
function pagereset(m) {
    switch (m){
        case 0:
            // getHtml('signalConditioning');
            break;
        case 1:
            // getHtml('antiFrictionNet');
            // get_antiwifi_status();
            antiShow=antiIntervalenable=0;
            break;
        case 2:
            // getHtml('childrenModel');
            childrenClear();
            break;
        case 3:
            // getHtml('visitorWifi');
            // getvisitorinfo();
            break;
        case 4:
            // getHtml('settingsTaught');
            // $('.settings-taught-content div').addClass('hide');
            // $('.settings-taught-content div').removeAttr('style');
            $('.configure-study').removeClass('hide');
            $('.configure-study .prepare').removeClass('hide');
            $('.configure-study .prepare').removeAttr('style');
            break;
        case 5:
            // getHtml('wifiInterval');
            intervalReset(1);
            break;
        case 6:
            // getHtml('firewallControl');
            break;
        case 7:
            // getHtml('qos');
            // getQosInfo();
            break;
        case 8:
            // getHtml('wirelessRepeater');
            getwdsstatus();
            break;
        case 9:
            // getHtml('vpnSetting');
            vpnSettingClear(2);
            clearTimeout(vpnrefreshInterval);
            vpnrefreshenable=0
            break;
        case 10:
            // getHtml('portMapped');
            vpsClear(2);
            break;
        case 11:
            // getHtml('customHost');
            // gethostsinfo();
            break;
        default:
            break;
    }
}
function spacebetween() {
    var i=$('.nav-list li').length;
    var n=0
    for(var j=0;j<i;){
        n=n+$('.nav-list li').eq(j)[0].scrollWidth;
        j++;
    }
    var m=Number(((1160-n)/i/2).toString().split('.')[0]);
    $('.nav-list li').attr('style','margin:0 '+m+'px;')
}
var antiShow=0;
$('.nav-list ul li').click(function () {
    var oldindex=$('.nav-list ul li a.active').parent().index();
    if(!$(this).children().hasClass('active')){
        $('.nav-list ul li a.active').removeClass('active');
        $(this).children().addClass('active');
    }
    var i=$(this).index();
    if(oldindex!=i){
        pagereset(oldindex);
    }
    if(i==1){
        // pagereset(i);
        antiShow=1;
    }
    if(!$('.frame-list>div').eq(i).hasClass('show')){
        $('.frame-list>div.show').hide();
        $('.frame-list>div.show').removeClass('show');
        $('.frame-list>div').eq(i).addClass('show');
        $('.frame-list>div').eq(i).fadeIn();
    }
    if($('.error-info').html().length>0){
        errorhide();
    }
    // oldindex=i;
})
$('.checkhide+span').click(function () {
    // if(!$(this).is(":checked")){
    //     $(this).removeAttr('checked');
    // }else {
    //     $(this).prop('checked','checked');
    // }
    $(this).prev().attr('checked',!$(this).prev().is(':checked'));
    if($('.error-info').html().length>0){
        errorhide();
    }
})
$('.nav-list ul li').click(function (){totop()});
function totop() {
    $('html,body').animate({scrollTop: '0px'}, 800);
}
function nonspace(e) {
    e.val(e.val().replace(/\s/g,''))
}
function waitingpopup() {
    $('.loading').fadeIn();
}
function waitingpopupclose() {
    $('.loading').fadeOut();
}
$('.signal-conditioning-icon').parent().click(function () {
    getConditioning();
})
function getConditioning() {
    var data={
        "requestUrl":"get_wifi_txpower",
        "method": "get"
    }
   postAjax(data,signalfill)
}
function signalfill(res) {
    if (res.errorcode == 0) {
        var i=res.data.mode;
        $('.signal-strength-select button').removeClass('selected');
        $('.mode-description>span').addClass('hide');
        $('.mode-description>span').eq(i).removeClass('hide');
        $('.mode-description>span').eq(i).fadeIn();
        switch(i){
            case 0:
                $('.efficient-mode').addClass('selected');
                break;
            case 1:
                $('.balanced-mode').addClass('selected');
                break;
            case 2:
                $('.performance-mode').addClass('selected');
                break;
            default:
                break;
        }
        strengthbar(i);
    }else {
        err.error_msg(res);
    }
}
$('.signal-strength-select>button').click(function () {
    resultTipshow(-1,-1);
    var i=$(this).index();
    if(i==2){
        i=1;
    }
    if(i==4){
        i=2;
    }
    // strengthbar(i);
    var data={
        "data":{
            "mode":i
        },
        "requestUrl":"set_wifi_txpower",
        "method": "post"
    }
    postAjax(data,signalset)
})
function signalset(res) {
    if (res.errorcode == 0) {
        resultTipshow(1,'保存成功');
        // $('.mode-description>span').attr('class','hide');
        // $('.mode-description>span').eq(i).removeClass('hide');
        // $('.mode-description>span').eq(i).fadeIn();
        // $('.signal-strength-select .selected').removeClass('selected');
        // $(selectbtn).addClass('selected')
        getConditioning();
    }
    else {
        err.error_msg(res);
    }
}
function strengthbar(e) {
    switch(e){
        case 0:
            // $('.strength-bar').attr('class','strength-bar efficient-mode-strength');
            strength_transition(45)
            break;
        case 1:
            // $('.strength-bar').attr('class','strength-bar balanced-mode-strength');
            strength_transition(320)
            break;
        case 2:
            // $('.strength-bar').attr('class','strength-bar');
            strength_transition(630)
            break;
        default:
            break;
    }
}
function strength_transition(n) {
    $('.strength-bar').animate({width:n},500)
}
$('.switch-icon').click(function settingSwitch() {
    $(this).prev().attr('checked',!$(this).prev().is(':checked'));
})
// 防蹭网
var antienable;
function get_antiwifi_status() {
    if(antiShow!=1){
        return;
    }
    var data={
        "requestUrl":"get_antiwifi_status",
        "method": "get"
    }
   postAjax(data,antistatusfill)
}
function antistatusfill(res) {
    if (res.errorcode == 0) {
        if(antiIntervalenable!=1){
            antienable=res.data.enable;
            if(res.data.enable==1){
                $('.question-selector').removeAttr('disabled');
                $('.answer').removeAttr('disabled');
                $('.limit-connect-router').removeAttr('disabled');
                $('.limit-connect-checked').removeAttr('disabled');
                $('.anti-input').attr('checked',true);
                if($('.anti-device-manage ').hasClass('hide')){
                    $('.anti-device-manage ').removeClass('hide');
                    $('.anti-device-manage ').fadeIn();
                }
                $('.question-input').removeAttr('disabled');
                // antiIntervalenable=1;
                // get_antiwifi_dev_list();
            }else {
                console.log(000)
                antiIntervalenable=0;
                $('.anti-device-manage ').addClass('hide');
                $('.anti-device-manage ').fadeOut();
                $('.anti-input').attr('checked',false);
                $('.description-form .form-item>input').attr('disabled','disabled');
                $('.question-input').attr('disabled','disabled');
                $('.question-selector').attr('disabled','disabled');
            }
            res.data.router_access==1? $('.limit-connect-router').attr('checked',true):$('.limit-connect-router').attr('checked',false);
            res.data.lan_dev_access==1? $('.limit-connect-checked').attr('checked',true):$('.limit-connect-checked').attr('checked',false);
        }
        if(res.data.enable==1){
            if(antiShow==1){
                antiIntervalenable=1;
            }else {
                return;
            }
            get_antiwifi_dev_list();
        }
    }else {
        err.error_msg(res);
    }
}
var antiInterval;
$('.anti-friction-net-icon').parent().click(function () {
    clearTimeout(antiInterval)
    getQuestion();
    get_antiwifi_status();
})
var $question,$questionIndex,$answer;
function getQuestion() {
    var data={
        "data":{
        },
        "requestUrl":"get_antiwifi_authinfo",
        "method": "get"
    }
   postAjax(data,questionfill)
}
function questionfill(res) {
    if (res.errorcode == 0) {
        var question=res.data.question;
        switch (question){
            case '':
                var n=0;
                break;
            case '我的手机号码后六位？':
                var n=0;
                break;
            case '我的微信号？':
                var n=1;
                break;
            case '我的QQ号码？':
                var n=2;
                break;
            default:
                var n=3;
                $('.question-item').removeClass('hide');
                $('.question-input').val(question);
                $question=question;
                break;
        }
        $questionIndex=n;
        $('.question-selector').val(n);
        $answer=res.data.answer;
        $('.answer').val($answer);
    }else {
        err.error_msg(res);
    }
}

// $(document).on('click','.anti-choose>span',function () {
//     if(!$(this).hasClass('active')){
//         var i= $('.anti-choose>span.active').index();
//         $('.anti-choose>span.active').removeClass('active');
//         $('.anti-content>div.hide').removeClass('hide');
//         $('.anti-content>div').eq(i).addClass('hide');
//         $(this).addClass('active');
//     }
// })
// $(document).on('click','.anti-manage',function () {
//     get_antiwifi_dev_list();
// })
function get_antiwifi_dev_list() {
    var antiDevices;
    var data={
        "data":{
            "mode":0
        },
        "requestUrl":"get_antiwifi_dev_list",
        "method": "get"
    }
    postAjax(data,antidevicelist)
}
function antidevicelist(res) {
    if (res.errorcode == 0) {
        antiDevices=res.data;
        for(var i=0;i<antiDevices.length;){
            antiDevices[i].status=0;
            i++;
        }
        pendingAuth(antiDevices);
    }else {
        err.error_msg(res);
    }
}
function pendingAuth(e) {
    var pendingAnti;
    var devices
    var data={
        "data":{
            "mode":1
        },
        "requestUrl":"get_antiwifi_dev_list",
        "method": "get"
    }
    $.ajax({
        type: 'POST',
        url: '/cgi-bin/web.cgi/',
        data: JSON.stringify(data),
        dataType: 'json',
        success: function (res) {
            if (res.errorcode == 0) {
                pendingAnti=res.data;
                for(var i=0;i<pendingAnti.length;){
                    pendingAnti[i].status=1;
                    i++;
                }
                devices=e.concat(pendingAnti);
                if(devices.length==0){
                    listinfo=[{"length":0}]
                }else {
                    listinfo = devices;
                }
                infolist(0)
                clearTimeout(antiInterval);
                if(antiIntervalenable==1&&antiShow==1){
                    antiInterval=setTimeout(function () {
                        // getQuestion();
                        get_antiwifi_status()
                    },5000);
                }
            }else {
                err.error_msg(res);
            }
        }
    })
}
$('.anti-switch-icon').click(function () {
    if($(this).prev().is(":checked")){
        $('.question-selector').removeAttr('disabled');
        $('.answer').removeAttr('disabled');
        $('.limit-connect-router').removeAttr('disabled');
        $('.limit-connect-checked').removeAttr('disabled');
        $('.limit-connect-router').attr('checked',true);
        $('.limit-connect-checked').attr('checked',true);
        if($('.question-selector').val()==3){
            $('.question-input').removeAttr('disabled');
            $('.question-item').removeClass('hide');
        }
    }else {
        $('.question-selector').prop('disabled','disabled');
        $('.answer').prop('disabled','disabled');
        $('.limit-connect-router').prop('disabled','disabled');
        $('.limit-connect-checked').prop('disabled','disabled');
        $('.limit-connect-router').attr('checked',false);
        $('.limit-connect-checked').attr('checked',false);
        if($('.question-selector').val()==$questionIndex){
            $('.answer').val($answer);
            if($questionIndex==3){
                $('.question-input').val($question);
            }
        }
        if($('.question-selector').val()==3){
            $('.question-input').prop('disabled','disabled');
            $('.question-input').prop('disabled','disabled');
        }
    }
})
$('.question-selector').change(function(){
    if($('.error-info').html().length>0){
        errorhide();
    }
    if($(this).val() == "3"){
        $('.question-item').removeClass('hide');
        $(".question-item").fadeIn();
        $('.question-input').removeAttr('disabled');
        $('.question-item').removeClass('hide');
        if($question==3){
            $('.question-input').val($question);
        }
    }else {
        $(".question-item").fadeOut();
        $('.question-item').addClass('hide');
        if(!$('.question-item').hasClass('hide')){
            $('.question-item').addClass('hide');
            $('.question-input').attr('disabled','disabled');
        }
        if($question==3){
            $('.question-input').val();
        }
    }
    if($('.question-selector').val()!=$questionIndex){
        $('.answer').val('');
    }else {
        $('.answer').val($answer);
    }
});
// $('.answer').on('input',function () {
//     if($('.question-selector').val()==0 || $('.question-selector').val()==2){
//         $('.answer').val($('.answer').val().replace(/\D/g,''))
//     }
// })
$('.anti-save').click(function () {
    if($('.anti-input').is(':checked')){
        if($('.question-selector').val()==3 ) {
            if($('.question-input').val().length==0 || $('.question-input').val().length>64) {
                errorShow('.question-input','请输入1-64位自定义问题');
                return;
            }
        }
        if($('.answer').val().length==0) {
            errorShow('.answer','请输入防蹭网问题答案');
            return;
        }
        if($('.question-selector').val()==0 ) {
            if(!(/^[0-9]{6}$/).test($('.answer').val())) {
                errorShow('.answer','请输入手机号后6位数');
                return;
            }
        }
        if($('.question-selector').val()==1) {
            if(!(/^[a-zA-Z]{1}[-_a-zA-Z0-9]{5,19}$/).test($('.answer').val())) {
                errorShow('.answer','请输入正确的微信号');
                return;
            }
        }
        if($('.question-selector').val()==2 ) {
            if(!(/^[1-9]{1}[0-9]{4,11}$/).test($('.answer').val())) {
                errorShow('.answer','请输入正确的qq号');
                return;
            }
        }
        if($('.question-selector').val()==3) {
            if($('.answer').val().length==0) {
                errorShow('.answer','请输入答案');
                return;
            }
        }
    }
    var question;
    var i=$('.question-selector').val();
    (i==3?question=$('.question-input').val():question=$('.question-selector>option').eq(i).text());
    var data = {
        "data": {
            "enable": $('.anti-input').is(":checked")==true?1:0,
            "question": question,
            "answer": $('.answer').val()
        },
        "requestUrl": "set_antiwifi",
        "method": "post"
    }
    resultTipshow(-1,-1);
    postAjax(data,anti_save)

})
function anti_save(res) {
    if (res.errorcode == 0) {
        if(!$('.anti-input').is(":checked")){
            $('.anti-device-manage').fadeOut();
            $('.anti-device-manage').addClass('hide');
        }else{
            $('.anti-device-manage').removeClass('hide');
            $('.anti-device-manage').fadeIn();
            // get_antiwifi_status();
            // if($('.anti-input').is(":checked")){
            // $('.question-selector').val(0);
            access();
        }
        resultTipshow(1,'保存成功');
    }else {
        err.error_msg(res);
    }
}
function access() {
    var data={
        "data":{
            "router_access": $('.limit-connect-router').is(":checked")?1:0,
            "lan_dev_access":$('.limit-connect-checked').is(":checked")?1:0,
        },
        "requestUrl":"set_antiwifi_admin_forbidden",
        "method": "post"
    }
    postAjax(data,access_save)
}
function access_save(res) {
    if (res.errorcode == 0) {
        get_antiwifi_status();
        resultTipshow(1,'保存成功');
    }else {
        err.error_msg(res);
    }
}
function abrogate(m) {
    resultTipshow(-1,-1);
    var data={
        "data":{
            "dev_mac":m,
            "auth":2,
            "block":0
        },
        "requestUrl":"set_authed_antiwifi_dev",
        "method": "post"
    }
    postAjax(data,sub);
}
function certificate(m) {
    resultTipshow(-1,-1);
    var data={
        "data":{
            "dev_mac":m,
            "auth":1,
            "block":0
        },
        "requestUrl":"set_authed_antiwifi_dev",
        "method": "post"
    }
    postAjax(data,sub);
}
function blackdevice(m,n) {
    resultTipshow(-1,-1);
    var data={
        "data":{
            "dev_name":m,
            "dev_mac":n,
            "auth":0,
            "block":1
        },
        "requestUrl":"set_authed_antiwifi_dev",
        "method": "post"
    }
    postAjax(data,sub);
}
function sub(res) {
    if (res.errorcode == 0) {
        resultTipshow(1,'保存成功');
        get_antiwifi_dev_list();
    }else {
        err.error_msg(res);
    }
}
$('.children-model-icon').parent().click(function () {
    getinfo();
})
$('.list-refresh-btn').click(function () {
    $(this).attr('disabled','disabled');
    getinfo();
    setTimeout(function () {
        $('.list-refresh-btn').removeAttr('disabled','disabled');
    },3000)
})
var childrenListInfo='';
function getinfo() {
    var data={
        "requestUrl": "proc_childrule_cfg",
        "method": "get"
    }
    postAjax(data,childrenlistget)
}
function childrenlistget(res) {
    if(res.errorcode==0)
    {
        var childMac=[];
        var data = res.data;
        if(data.length==0){
            data=[{"length":0}]
        }else {
            for(var i=0;i<data.length;){
                childMac[i]=data[i].macaddr;
                var n=data[i].weekdays.split(' ');
                data[i].weekdaysCN='';
                if(n.length==7){
                    data[i].weekdaysCN='每天';
                }else{
                    for(var j=0;j<n.length;){
                        switch (Number(n[j])){
                            case 1:
                                data[i].weekdaysCN= data[i].weekdaysCN+'星期一 ';
                                break;
                            case 2:
                                data[i].weekdaysCN= data[i].weekdaysCN+'星期二 ';
                                break;
                            case 3:
                                data[i].weekdaysCN= data[i].weekdaysCN+'星期三 ';
                                break;
                            case 4:
                                data[i].weekdaysCN= data[i].weekdaysCN+'星期四 ';
                                break;
                            case 5:
                                data[i].weekdaysCN= data[i].weekdaysCN+'星期五 ';
                                break;
                            case 6:
                                data[i].weekdaysCN= data[i].weekdaysCN+'星期六 ';
                                break;
                            case 7:
                                data[i].weekdaysCN= data[i].weekdaysCN+'星期日 ';
                                break;
                            default:
                                break;
                        }
                        j++;
                    }
                }
                i++;
            }
        }
        childrenListInfo=data;
        // var compiled = _.template(document.getElementById("childListTemplate").innerHTML);
        // var str = compiled(data);
        // $('.children-list').html(str);
        deviceListNew(childMac);
    }else {
        err.error_msg(res);
    }
}
function deviceListNew(childMac) {
    var data={
        "requestUrl": "get_stalist_cfg",
        "method": "get"
    }
    $.ajax({
        type:'POST',
        url:'/cgi-bin/web.cgi/',
        data:JSON.stringify(data),
        dataType:'json',
        success:function (res) {
            if(res.errorcode==0)
            {
                for(var i=0;i<res.data.stalist.length;){
                    var t=res.data.stalist[i].stastatus;
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
                    res.data.stalist[i].ownip=res.data.ownip;
                    i++;
                }
                var data = res.data.stalist;
                var m=childMac.length;
                var n=data.length;
                var l=0;
                for(var i=0;i<n;i++){
                    if(m>0){
                        data[i].enabled=1;
                        for(var j=0;j<m;j++){
                            if(data[i].macaddr==childMac[j] || data[i].stastatus==0){
                                data[i].enabled=0;
                            }
                        }
                    }else {
                        if(data[i].stastatus>0){
                            data[i].enabled=1;
                        }else {
                            data[i].enabled=0;
                        }
                    }
                    if(data[i].enabled==0){
                        l++;
                    }
                    for(var k=0;k<childrenListInfo.length;k++){
                        if(data[i].macaddr==childrenListInfo[k].macaddr){
                            childrenListInfo[k].staname=data[i].staname;
                        }
                    }
                }
                // for(var i=0;i<n;i++){
                //
                // }
                if(l==n){
                    data=[{"length":0}]
                }
                listinfo = childrenListInfo;
                infolist(1);
                var compiled = _.template(document.getElementById("deviceListTemplate").innerHTML);
                var str = compiled(data.sort(function (a,b) {
                    return a.stastatus-b.stastatus;
                }));
                $('.device-list').html(str);
            }else {
                err.error_msg(res);
            }
        }
    })
}
var inputVal='';
$('.timelist input').focus(function () {
    inputVal= $(this).val();
    $(this).val('');
})
$('.timelist input').blur(function () {
    if($(this).val().length==0){
        $(this).val(inputVal);
    }else {
        if($(this).val().length==1){
            var n=$(this).val()
            $(this).val(0+n);
        }
        // $(this).val($(this).val().replace(/\D/g,''))
    }
})
$('.timelist input').on('input',function () {
    $(this).val($(this).val().replace(/\D/g,''))
    if($(this).hasClass('hour')){
        if($(this).val()>23){
           $(this).val(23)
        }
    }
    if($(this).hasClass('minute')){
        if($(this).val()>59){
            $(this).val(59)
        }
    }
    var tip=$(this).parent().parent().children().find('.form-tip').children();
    if(tip.html()!=''){
        tip.html('');
    }
})
// function hourReg(e){
//     var reg=/^([0-1]{1}\d|2[0-3]\d)$/;
//     if(!reg.test(e)){
//         e.replace(/\D/g,'');
//     }
// }
// function minuteReg(e){
//     var reg="^(([0-5]d)$";
//     if(!reg.test(e)){
//         e.replace(/\D/g,'')
//     }
// }
// $('.hour-start').on('input',function () {
//     var e=$(this).val();
//     if(e.length==1){
//         if(Number(e)>2){
//             $(this).val('0'+e);
//         }else {
//             $(this).val(e);
//         }
//     }else {
//         if(e.length==2){
//             if(Number(e[1])>3){
//                 $(this).val(e[0]);
//             }else {
//                 $(this).val(e);
//             }
//         }
//     }
    // var reg=/^(([0-1]{1}\d{1}|[2]{1}[0-3]{1}))$/;
    // if(!reg.test($(this).val())){
    //     $(this).val('')
    // }
// })
// $('.hour-end').on('input',function () {
//     var reg=/^([0-1]{1}\d|2[0-3])$/;
//     if(!reg.test($(this).val())){
//         $(this).val('')
//     }
// })
// $('.minute-start').on('input',function () {
//     var reg=/^([0-5]{1}\d)$/;
//     if(!reg.test($(this).val())){
//         $(this).val('')
//     }
// })
// $('.hour-start').on('input',function () {
//     var reg=/^([0-5]{1}\d)$/;
//     if(!reg.test($(this).val())){
//         $(this).val('')
//     }
// })
// $('.minute-end').click(function () {
//     minuteReg($(this).val());
// })
var childsetmac='';
function childrenset(m,n) {
    childsetmac=m;
    $('.child-name').html(n);
    childEdit();
    $('.child-save').removeClass('hide');
    $('.child-modify').addClass('hide');
}
function childrenClear() {
    $('.children-add').hide();
    $('.children-info-list').fadeIn();
    $('.child-name').html('');
    $('.hour-start').val('00');
    $('.minute-start').val('00');
    $('.hour-end').val('00');
    $('.minute-end').val('00');
    $('.child-add-choose input').attr('checked','checked');
    if(!$('.child-modify').hasClass('hide')){
        $('.child-modify').addClass('hide');
        $('.child-save').removeClass('hide');
    }
    errorhide()
}
$('.child-add-cancel').click(function () {
    childrenClear();
})
$('.child-save').click(function () {
    if($('.hour-start').val()>23 || $('.hour-end').val()>23){
        errorShow('.minute-end+label','小时输入不正确');
        return;
    }
    if($('.timelist input').val().match(/\D/g,'')){
        errorShow('.timelist .minute-end+label','时间不支持非数字字符')
        return
    }
    if($('.timelist .hour').val()>23){
        errorShow('.timelist .minute-end+label','小时区间为0-23')
        return
    }
    if($('.timelist .minute').val()>59){
        errorShow('.timelist .minute-end+label','分钟区间为0-59')
        return
    }
    if($('.hour-start').val()==$('.hour-end').val() && $('.minute-start').val()== $('.minute-end').val()){
        errorShow('.minute-end+label','开始时间和结束时间不能相同');
        return;
    }
    if($('.minute-start').val()>59 || $('.minute-end').val()>59){
        errorShow('.minute-end+label','分钟输入不正确');
        return;
    }
    var weeksChecked=$('.child-add-choose input');
    var weeks='';
    for(var i=0;i<weeksChecked.length;){
        if(weeksChecked.eq(i).is(":checked")){
            weeks=weeks+(i+1)+' ';
            i++
        }else {i++;}
    }
    if(weeks!='') {
        weeks = weeks.substr(0, weeks.length - 1);
    }else {
        errorShow('.child-add-choose .sunday','请选择重复周期');
        return;
    }
    var data={
        "data":{
            "macaddr": childsetmac,
            "start_time":$('.hour-start').val()+':'+$('.minute-start').val(),
            "stop_time":$('.hour-end').val()+':'+$('.minute-end').val(),
            "weekdays":weeks,
            "enabled": 1
        },
        "requestUrl":"proc_childrule_cfg",
        "method": "post"
    }
    resultTipshow(-1,-1);
   postAjax(data,child_reg_save);
})
function child_reg_save(res) {
    if (res.errorcode == 0) {
        resultTipshow(1,'添加成功');
        getinfo(0);
        childrenClear();
    }else {
        err.error_msg(res);
    }
}
function childStatusChange(a,b,c,d,e) {
    // $('.loading').fadeIn()
    resultTipshow(-1,-1);
    var mac=b;
    var start=c;
    var end=d;
    var weeks=e;
    if(a==0){
        var oldenabled=1;
        var newenabled=0;
    }else {
        var oldenabled=0;
        var newenabled=1;
    }
    var data={
        "data": {
            "oldmacaddr": mac,
            "oldstarttime":start,
            "oldstoptime":end,
            "oldweekdays":weeks,
            "oldenabled": oldenabled,
            "newmacaddr": mac,
            "newstarttime":start,
            "newstoptime":end,
            "newweekdays":weeks,
            "newenabled": newenabled
        },
        "requestUrl":"proc_childrule_cfg",
        "method": "put"
    }
    postAjax(data,childStatusModify_Sub);
}
var childDeviceOldInfo='';
function childStatusModify(a,b,c,d,e) {
    var mac=a;
    var start=b;
    var end=c;
    var weeks=d;
    $('.child-name').html(e);
    $('.hour-start').val(start.split(':')[0]);
    $('.minute-start').val(start.split(':')[1]);
    $('.hour-end').val(end.split(':')[0]);
    $('.minute-end').val(end.split(':')[1]);
    var m=weeks.split(' ');
    var n=$('.child-add-choose input');
    numToWeeks(m,n);
    for(var i=0;i<childrenListInfo.length;){
        if(childrenListInfo[i].macaddr==mac){
            var oldenabled=childrenListInfo[i].enabled;
            i++;
            break;
        }else {
            i++;
        }
    }
    var data={
        "oldmacaddr": mac,
        "oldstarttime":start,
        "oldstoptime":end,
        "oldweekdays":weeks,
        "oldenable": oldenabled,
    }
    childDeviceOldInfo=data;
    childEdit();
    $('.child-save').addClass('hide');
    $('.child-modify').removeClass('hide');
}
function childEdit() {
    $('.children-info-list').hide();
    $('.children-add').fadeIn();
    totop();
}
$('.interval-modify-cancel').click(function () {
    $('.interval-modify').addClass('hide');
})
function numToWeeks(m,n) {
    n.removeAttr('checked')
    for(var i=0;i<m.length;){
        n.eq(Number(m[i])-1).attr('checked','checked');
        i++;
    }
}
function weeksToNum(m) {
    var n='';
    for(var i=0;i<m.length;){
        if(m.eq(i).is(":checked")){
            n=n+(i+1)+' ';
            i++
        }else {i++;}
    }
    if(n!=''){
        n=n.substr(0,n.length-1);
        return n;
    }else {
        return n;
    }
}
$('.child-modify').click(function () {
    if($('.hour-start').val()>23 || $('.hour-end').val()>23){
        errorShow('.minute-end+label','小时输入不正确');
        return;
    }
    if($('.hour-start').val()==$('.hour-end').val() && $('.minute-start').val()==$('.minute-end').val()){
        errorShow('.minute-end+label','开始时间和结束时间不能相同');
        return;
    }
    if($('.minute-start').val()>59 || $('.minute-end').val()>59){
        errorShow('.minute-end+label','分钟输入不正确');
        return;
    }
    weeksToData(weeksToNum);
})
function weeksToData(callback) {
    var m=$('.child-add-choose input');
    var t=callback(m);
    if(t==''){
        errorShow('.child-add-choose','重复周期未选择');
        return;
    }
    resultTipshow(-1,-1);
    var data={
        "data":{
            "oldmacaddr": childDeviceOldInfo.oldmacaddr,
            "oldstarttime":childDeviceOldInfo.oldstarttime,
            "oldstoptime":childDeviceOldInfo.oldstoptime,
            "oldweekdays":childDeviceOldInfo.oldweekdays,
            "oldenabled": childDeviceOldInfo.oldenable,
            "newmacaddr": childDeviceOldInfo.oldmacaddr,
            "newstarttime":$('.hour-start').val()+':'+$('.minute-start').val(),
            "newstoptime":$('.hour-end').val()+':'+$('.minute-end').val(),
            "newweekdays":t,
            "newenabled": childDeviceOldInfo.oldenable
        },
        "requestUrl":"proc_childrule_cfg",
        "method": "put"
    }
   postAjax(data,child_reg_modify)
}
function child_reg_modify(res) {
    if (res.errorcode == 0) {
        resultTipshow(1,'修改成功');
        childrenClear();
        getinfo();
    }else{
        err.error_msg(res);
    }
}
function childStatusModify_Sub(res) {
    if (res.errorcode == 0) {
        // $('.loading').fadeOut()
        resultTipshow(1,'修改成功');
        getinfo();
    }else {
        err.error_msg(res);
    }
}
function childRmoeve(a) {
    resultTipshow(-1,-1);
    var mac=a;
    // var start=b;
    // var end=c;
    // var weeks=d;
    // for(var i=0;i<childrenListInfo.length;){
    //     if(childrenListInfo[i].macaddr==mac){
    //         var enabled=childrenListInfo[i].enabled;
    //         i++;
    //         break;
    //     }else {
    //         i++;
    //     }
    // }
    var data={"data": {
            "macaddr": mac,
            // "start_time":start,
            // "stop_time":end,
            // "weekdays":weeks,
            // "enabled": enabled
        },
        "requestUrl": "proc_childrule_cfg",
        "method": "delete"
    }
   postAjax(data,chil_reg_del,disLoading)
}
function chil_reg_del(res) {
    if (res.errorcode == 0) {
        resultTipshow(1,'删除成功');
        getinfo();
    }else {
        err.error_msg(res);
    }
}
$('.visitor-wifi-icon').parent().click(function () {
    getvisitorinfo();
})
function getvisitorinfo() {
    var data={
        "requestUrl": "get_guest_wifi",
        "method": "get"
    };
    postAjax(data,visitor_info_fill)
}
function visitor_info_fill(res) {
    if (res.errorcode == 0) {
        $('.visitor-ssid').val(res.data.name);
        if($('.visitor-ssid').val().length==0){
            $('.visitor-ssid').val('3Care-访客wifi');
        }
        if (res.data.isHide == 1) {
            $('.hide-visitor-wifi-checked').attr('checked',true);
        }else {
            $('.hide-visitor-wifi-checked').attr('checked',false);
        }
        if (res.data.enable == 1) {
            $('.visitor-input').attr('checked',true);
            $('.visitor-ssid').removeAttr('disabled');
            $('.hide-visitor-wifi-checked').removeAttr('disabled');
            var n = res.data.userNum;
            $('.online-num').html('在线设备数量：'+n);
        }else {
            $('.online-num').html('');
            $('.visitor-input').attr('checked',false);
            $('.visitor-ssid').attr('disabled','disabled');
            $('.hide-visitor-wifi-checked').attr('disabled','disabled');
        }
        if($('.visitor-input').is(':checked')){
            $('.visitor-form').removeClass('hide');
        }
    }else {
        err.error_msg(res);
    }
}
$('.visitor-switch').click(function () {
    if($('.visitor-input').is(':checked')){
        $('.visitor-ssid').removeAttr('disabled');
        $('.hide-visitor-wifi-checked').removeAttr('disabled');
    }else {
        if($('.visitor-ssid').val().length==0){
            $('.visitor-ssid').val('QTEC-访客wifi');
        }
        $('.visitor-ssid').attr('disabled','disabled');
        $('.hide-visitor-wifi-checked').attr('disabled','disabled');
    }
})

$('.visitor-limit').click(function () {
    $('.visitor-limit-form').toggleClass('hide','');
})

$('.visitor-ssid').on('input',function () {
    if(countnums($('.visitor-ssid').val())>32){
        errorShow('.visitor-ssid','访客wifi名称长度超出限制');
    }
})
$('.visitor-save').click(function () {
    var ssid=$('.visitor-ssid').val();
    if($('.visitor-input').is(':checked')){
        if(ssid.length==0){
            errorShow('.visitor-ssid','访客wifi名称不能为空');
            return;
        }
        if(countnums($('.visitor-ssid').val())>32){
            // $('.visitor-ssid').val($('.visitor-ssid').val().substr(0,))
            errorShow('.visitor-ssid','访客wifi名称长度超出限制');
            return;
        }else {
            if($('.hide-visitor-wifi-checked').is(':checked')){
                var hide=1;
            }else {
                var hide=0;
            }
        }
    }
    if(ssid.length>0){
        var data={
            "data":{
                "enable":$('.visitor-input').is(':checked')==true?1:0,
                "name":ssid,
                "isHide":hide
            },
            "requestUrl": "set_guest_wifi",
            "method": "post"
        }
    }
    resultTipshow(-1,-1);
   postAjax(data,visitor_set)
})
function visitor_set(res) {
    if (res.errorcode == 0) {
        resultTipshow(1,'保存成功，wifi将会重启');
        getvisitorinfo();
    }else {
        err.error_msg(res);
    }
}
var studyStatus=0;
$('.configure-save').click(function () {
    resultTipshow(-1,-1);
    $('.progress-loading').removeClass('hide');
    var data={
        "requestUrl": "onekeyswitch",
        "method": "post"
    }
    postAjax(data,studying_start)
})
function studying_start(res) {
    resultTiphide();
    if (res.errorcode == 0) {
        $('.prepare').fadeOut();
        $('.prepare').addClass('hide');
        $('.studying').removeClass('hide');
        $('.studying').fadeIn();
        studyStatus=1;
        get_onekeyswitch();
    }else {
        $('.progress-loading').addClass('hide');
        err.error_msg(res);
    }
}
var onekeyswitchInterval;
function get_onekeyswitch(e) {
    if(!e){
        var e=0;
    }
    var data={
        "requestUrl": "get_onekeyswitch",
        "method": "get"
    }
    $.ajax({
        type: 'POST',
        url: '/cgi-bin/web.cgi/',
        data: JSON.stringify(data),
        dataType: 'json',
        success: function (res) {
            if (res.errorcode == 0) {
                if(e<15 ){
                    e++;
                    clearTimeout(onekeyswitchInterval)
                    if(studyStatus==1){
                        onekeyswitchInterval=setTimeout(function () {
                            get_onekeyswitch(e);
                        },2000)
                    }
                }else {
                    res.data.status==5;
                    // clearTimeout(onekeyswitchInterval);
                }
                if(studyStatus==1) {
                    studingstatus(res.data.status);
                }
            }else {
                $('.progress-loading').addClass('hide');
                err.error_msg(res);
            }
        }
    })
}
function studingstatus(e) {
    switch (e){
        case 1:
            $('.studying-tip').html("正在复制配置请稍候...");
            break;
        case 2:
            $('.studying-tip').html("宽带帐号密码获取成功，正在设置...");
            break;
        case 3:
            clearTimeout(onekeyswitchInterval);
            studyStatus=0;
            $('.studying-tip').html("宽带帐号密码设置成功，正在重启...");
            $('.restart-loading').removeClass('hide');
            reboot();
            break;
        case 4:
            clearTimeout(onekeyswitchInterval);
            studyStatus=0;
            $('.study-failed-tip').html("宽带帐号或密码无效");
            $('.studying').fadeOut();
            $('.studying').addClass('hide');
            $('.study-failed').removeClass('hide');
            $('.study-failed').fadeIn();
            break;
        case 5:
            clearTimeout(onekeyswitchInterval);
            studyStatus=0;
            $('.study-failed-tip').html("已超时，学习失败，请检查设备连接情况");
            $('.studying').fadeOut();
            $('.studying').addClass('hide');
            $('.study-failed').removeClass('hide');
            $('.study-failed').fadeIn();
            break;
        case 6:
            clearTimeout(onekeyswitchInterval);
            studyStatus=0;
            $('.study-success-tip').html("配置自学已完成");
            $('.studying').fadeOut();
            $('.studying').addClass('hide');
            $('.study-success').removeClass('hide');
            $('.study-success').fadeIn();
            break;
        default:
            break;
    }
}
function reboot() {
    var t=81;
    for (var i = 0; i <= 81; i++) {
        window.setTimeout(function () {
            if (t > 1) {
                t--;
                $('.reboot-cutdown').html(t);
            } else {
                $('.restart-loading').addClass('hide');
                onekeyswitchInterval;
            }
        }, i * 1000)
    }
}
$('.studying-cancel').click(function () {
    $('.progress-loading').addClass('hide');
    clearTimeout(onekeyswitchInterval);
    studyStatus=0;
    $('.prepare').removeClass('hide');
    $('.prepare').fadeIn();
    $('.studying').fadeOut();
    $('.studying').addClass('hide');
})
$('.studying-failed-confirm').click(function () {
    $('.progress-loading').addClass('hide');
    $('.study-failed').fadeOut();
    $('.study-failed').addClass('hide');
    $('.prepare').removeClass('hide');
    $('.prepare').fadeIn();
})
$('.studying-success-confirm').click(function () {
    $('.progress-loading').addClass('hide');
    $('.study-success').fadeOut();
    $('.study-success').addClass('hide');
    $('.prepare').removeClass('hide');
    $('.prepare').fadeIn();
})

$('.wifi-interval-icon').parent().click(function () {
    getIntervalList();
})
var intervalInfo='';
function getIntervalList() {
    var data={
        "requestUrl": "get_wifi_timer",
        "method": "get"
    }
    postAjax(data,wifi_interval_list)
}
function wifi_interval_list(res) {
    if (res.errorcode == 0) {
        var data = res.data.rules;
        if(data.length==0){
            data=[{"length":0}]
        }else {
            intervalInfo=data;
            // if(e==1){
            //     wifiIntervalEffect();
            // }
            for(var i=0;i<data.length;){
                if(data[i].start_hour<10){
                    data[i].start_hour='0'+data[i].start_hour;
                }
                if(data[i].start_min<10){
                    data[i].start_min='0'+data[i].start_min;
                }
                if(data[i].stop_hour<10){
                    data[i].stop_hour='0'+data[i].stop_hour;
                }
                if(data[i].stop_min<10){
                    data[i].stop_min='0'+data[i].stop_min;
                }
                // childMac[i]=data[i].macaddr;
                var n=data[i].week_day.split(',');
                data[i].weekdaysCN='';
                if(n.length==7){
                    data[i].weekdaysCN='每天';
                }
                else {
                    for(var j=0;j<n.length;){
                        switch (Number(n[j])){
                            case 0:
                                data[i].weekdaysCN= data[i].weekdaysCN+'星期日 ';
                                break;
                            case 1:
                                data[i].weekdaysCN= data[i].weekdaysCN+'星期一 ';
                                break;
                            case 2:
                                data[i].weekdaysCN= data[i].weekdaysCN+'星期二 ';
                                break;
                            case 3:
                                data[i].weekdaysCN= data[i].weekdaysCN+'星期三 ';
                                break;
                            case 4:
                                data[i].weekdaysCN= data[i].weekdaysCN+'星期四 ';
                                break;
                            case 5:
                                data[i].weekdaysCN= data[i].weekdaysCN+'星期五 ';
                                break;
                            case 6:
                                data[i].weekdaysCN= data[i].weekdaysCN+'星期六 ';
                                break;
                            default:
                                break;
                        }
                        j++;
                    }
                }
                i++;
            }
        }
        listinfo = data;
        infolist(2);
    }else {
        err.error_msg(res);
    }
}
$('.effect-immediate input').click(function () {
    $(this).is(":checked")? $('.effect-immediate label').text('立即生效'):$('.effect-immediate label').text('暂不生效');
})
function timeCheck(a,b,c,d) {
    var e='';
    if(a==c && b==d){
        e='开始时间和结束时间不能相同';
        return e;
    }
    if(a==c && d-b<=5 && d>b){
        e='开始时间和结束时间间隔需大于5分钟';
        return e;
    }
    return e;
}
function intervalWeeksToNum(m) {
    var n='';
    for(var i=0;i<m.length;){
        if(m.eq(i).is(":checked")){
            if(i==6){
                n=n+','+0;
            }else {
                n=n+','+(i+1);
            }
            i++
        }else {i++;}
    }
    if(n.length>0){
        n=n.substr(1,n.length);
        return n;
    }else {
        n=-1;
        return n;
    }
}
$('.interval-add').click(function (callback) {
    var n=$('.interval-description').val();
    if(n.replace(/(^\s*)|(\s*$)/g,"").length==0){
        errorShow('.interval-description','描述不能为空');
        return;
    }
    if(n.length>16){
        errorShow('.interval-description','请输入1-16位描述');
        return;
    }
    intervalSettings(timeCheck);
})

function intervalSettings(callback) {
    var a=$('.interval-hour-start').val();
    var b=$('.interval-hour-end').val();
    var c=$('.interval-minute-start').val();
    var d=$('.interval-minute-end').val();
    var e=callback(a,b,c,d);
    if(e!=''){
        errorShow('.interval-minute-end+label',e);
    }else {
        intervalSub(intervalWeeksToNum);
    }
}
function intervalSub(callback) {
    var m=$('.add-interval-time input');
    var t=callback(m);
    if(t==-1){
        errorShow('.frame-list .show .sunday','请选择重复周期');
        return;
    }
    resultTipshow(-1,-1);
    var data={
        "data":{
            "enable":1,
            "rules":[]
        },
        "requestUrl": "set_wifi_timer",
        "method": "post"
    }
    var self=this;
    $.ajax({
        type: 'POST',
        url: '/cgi-bin/web.cgi/',
        data: JSON.stringify(data),
        dataType: 'json',
        success: function (res) {
            if (res.errorcode==0) {
                var data={
                    "data":{
                        "rule_enable":1,
                        "name":$('.interval-description').val(),
                        "start_hour":Number($('.interval-hour-start').val()),
                        "start_min":Number($('.interval-minute-start').val()),
                        "stop_hour":Number($('.interval-hour-end').val()),
                        "stop_min":Number($('.interval-minute-end').val()),
                        "week_day":t
                    },
                    "requestUrl": "set_wifi_timer_rule",
                    "method": "post"
                }
                $.ajax({
                    type: 'POST',
                    url: '/cgi-bin/web.cgi/',
                    data: JSON.stringify(data),
                    dataType: 'json',
                    success: function (res) {
                        if (res.errorcode==0) {
                            intervalReset(1);
                            getIntervalList();
                            resultTipshow(1, '保存成功');
                        }else {
                            err.error_msg(res);
                        }
                    }
                })
            }else {
                err.error_msg(res);
            }
        }
    })

}
function wifiIntervalEffect() {
    // var data={
    //     "requestUrl": "get_wifi_timer",
    //     "method": "get"
    // }
    // var childMac=[];
    // $.ajax({
    //     type: 'POST',
    //     url: '/cgi-bin/web.cgi/',
    //     data: JSON.stringify(data),
    //     dataType: 'json',
    //     success: function (res) {
    //         if (res.errorcode == 0) {
    //             intervalInfo= res.data.rules;
    //         }else {
    //             $('.success').removeClass('hide');
    //             $('.success-tip').html(res.msg);
    //             setTimeout(function () {
    //                 $('.success').addClass('hide');
    //                 $('.success-tip').html('');
    //             },2000)
    //         }
    //     }
    // })
    var data={
        "data":{
            "enable":1,
            "rules":intervalInfo
        },
        "requestUrl": "set_wifi_timer",
        "method": "post"
    }
    // waitingpopup();
    postAjax(data,wifi_interval_modify)
}
function wifi_interval_modify(res) {
    // waitingpopupclose();
    if (res.errorcode == 0) {
        resultTipshow(1,'修改成功');
        getIntervalList();
    }else {
        err.error_msg(res);
    }
}
function closeInterval(m) {
    errorhide()
    resultTipshow(-1,-1);
    var data={
        "data":{
            "enable":1,
            "rules":[
                {
                    "id":Number(m),
                    "rule_enable":0
                },
            ]
        },
        "requestUrl": "set_wifi_timer",
        "method": "post"
    }
   postAjax(data,wifi_interval_close)
}
function wifi_interval_close(res) {
    if (res.errorcode == 0) {
        intervalReset(1);
        resultTipshow(1,'关闭成功');
        getIntervalList();
    }else {
        err.error_msg(res);
    }
}
function openInterval(m) {
    errorhide()
    resultTipshow(-1,-1);
    var data={
        "data":{
            "enable":1,
            "rules":[
                {
                    "id":Number(m),
                    "rule_enable":1
                },
            ]
        },
        "requestUrl": "set_wifi_timer",
        "method": "post"
    }
   postAjax(data,wifi_interval_open)
}
function wifi_interval_open(res) {
    if (res.errorcode == 0) {
        intervalReset(1);
        resultTipshow(1,'开启成功');
        getIntervalList();
    }else {
        err.error_msg(res);
    }
}
function delInterval(m) {
    errorhide()
    resultTipshow(-1,-1);
    var data={
        "data":{
            "id":Number(m)
        },
        "requestUrl": "del_wifi_timer_rule",
        "method": "post"
    }
   postAjax(data,wifi_interval_del)
}
function wifi_interval_del(res) {
    if (res.errorcode == 0) {
        intervalReset(1);
        resultTipshow(1,'删除成功');
        getIntervalList();
    }else {
        err.error_msg(res);
    }
}
function intervalnumToWeeks(m,n) {
    for(var i=0;i<m.length;){
        n.eq(Number(m[i])-1).attr('checked','checked');
        i++;
    }
}
$(document).on('click','.modify-cancel',function () {
    intervalReset(1);
})
var modifyIntervalId='',modifyenabled='';
function modifyInterval(m) {
    errorhide()
    $('.interval-add').addClass('hide');
    $('.interval-modify-btn').removeClass('hide');
    modifyIntervalId=Number(m);
    for(var i=0;i<intervalInfo.length;){
        if(intervalInfo[i].id==modifyIntervalId){
            modifyenabled=intervalInfo[i].rule_enable;
            $('.interval-description').val(intervalInfo[i].name);
            $('.interval-hour-start').val(intervalInfo[i].start_hour)
            $('.interval-minute-start').val(intervalInfo[i].start_min)
            $('.interval-hour-end').val(intervalInfo[i].stop_hour)
            $('.interval-minute-end').val(intervalInfo[i].stop_min);
            var m=intervalInfo[i].week_day.split(',');
            var n=$('.add-interval-time input');
            n.removeAttr('checked');
            intervalnumToWeeks(m,n);
            i++;
            break;
        }
        else {
            i++;
        }
    }
    totop();
}
$('.wifi-interval-modify').click(function () {
    var n=$('.interval-description').val();
    if(n.replace(/(^\s*)|(\s*$)/g,"").length==0){
        errorShow('.interval-description','描述不能为空');
        return;
    }
    if(n.length>16){
        errorShow('.interval-description','请输入1-16位描述');
        return;
    }
    intervalModifySettings(timeCheck);
})
function intervalReset(e) {
    $('.effect-immediate input').attr("checked","checked");
    $('.interval-description').val('');
    $('.interval-hour-start').val('00')
    $('.interval-minute-start').val('00')
    $('.interval-hour-end').val('00')
    $('.interval-minute-end').val('00');
    var n=$('.add-interval-time input');
    n.attr('checked',true);
    if(e==1){
        $('.interval-add').removeClass('hide')
        $('.interval-modify-btn').addClass('hide');
    }
    errorhide()
}
function intervalModifySettings(callback) {
    var a=$('.interval-hour-start').val();
    var b=$('.interval-hour-end').val();
    var c=$('.interval-minute-start').val();
    var d=$('.interval-minute-end').val();
    var e=callback(a,b,c,d);
    if(e!=''){
        resultTipshow(0,e);
    }else {
        intervalModifySub(intervalWeeksToNum);
    }
}
function intervalModifySub(callback) {
    var m=$('.add-interval-time input');
    var t=callback(m);
    if(t==-1){
        errorShow('.frame-list .show .sunday','请选择重复周期');
        return;
    }
    var data={
        "data":{
            "id":Number(modifyIntervalId),
            "rule_enable":modifyenabled,
            "name":$('.interval-description').val(),
            "start_hour":Number($('.interval-hour-start').val()),
            "start_min":Number($('.interval-minute-start').val()),
            "stop_hour":Number($('.interval-hour-end').val()),
            "stop_min":Number($('.interval-minute-end').val()),
            "week_day":t
        },
        "requestUrl": "set_wifi_timer_rule",
        "method": "post"
    }
    resultTipshow(-1,-1);
    postAjax(data,interval_modify_sub)
}
function interval_modify_sub(res) {
    if (res.errorcode == 0) {
        intervalReset(1);
        resultTipshow(1,'修改成功');
        getIntervalList();
    }else {
        err.error_msg(res);
    }
}
$('.firewall-control-icon').parent().click(function () {
    getfirewallstatus();
});
function getfirewallstatus() {
    var data={
        "requestUrl": "get_firewall_status",
        "method": "get"
    }
   postAjax(data,firewall_status_list)
}
function firewall_status_list(res) {
    if (res.errorcode == 0) {
        firewallStatusUpdata(res);
    }else {
        err.error_msg(res);
    }
}
$('.turn-on-all').click(function () {
    var data={
        "data":{
            "url_firewall":1,
            "dns_hijack_firewall":1,
            "family_firewall":1,
            "password_firewall":1,
        },
        "requestUrl": "set_firewall_status",
        "method": "post"
    }
    firewallSwitch(data);
})
$('.turn-off-all').click(function () {
    var data={
        "data":{
            "url_firewall":0,
            "dns_hijack_firewall":0,
            "family_firewall":0,
            "password_firewall":0,
        },
        "requestUrl": "set_firewall_status",
        "method": "post"
    }
    firewallSwitch(data);
})
$('.firewall-icon').click(function () {
    var data={
        "data":{
            "url_firewall":$('.route-firewall-input').eq(0).is(":checked")==true?1:0,
            "dns_hijack_firewall":$('.route-firewall-input').eq(1).is(":checked")==true?1:0,
            "family_firewall":$('.route-firewall-input').eq(2).is(":checked")==true?1:0,
            "password_firewall":$('.route-firewall-input').eq(3).is(":checked")==true?1:0,
        },
        "requestUrl": "set_firewall_status",
        "method": "post"
    }
    firewallSwitch(data);
})
function firewallStatusUpdata(data) {
    var i=0;
    if(data.data.url_firewall == 1 ){
        i++;
        $('.route-firewall-input').eq(0).attr('checked', true);
        $('.url-firewall .firewall-status').html('保护中');
    }else{
        $('.route-firewall-input').eq(0).attr('checked', false);
        $('.url-firewall .firewall-status').html('已关闭');
    }
    if(data.data.dns_hijack_firewall == 1 ){
        i++;
        $('.route-firewall-input').eq(1).attr('checked', true);
        $('.dns-hijack-firewall .firewall-status').html('保护中');
    }else{
        $('.route-firewall-input').eq(1).attr('checked', false);
        $('.dns-hijack-firewall .firewall-status').html('已关闭');
    }
    if(data.data.family_firewall == 1 ){
        i++
        $('.route-firewall-input').eq(2).attr('checked', true);
        $('.family-firewall .firewall-status').html('保护中');
    }else{
        $('.route-firewall-input').eq(2).attr('checked', false);
        $('.family-firewall .firewall-status').html('已关闭');
    }
    if(data.data.password_firewall == 1 ){
        i++;
        $('.route-firewall-input').eq(3).attr('checked', true);
        $('.password-firewall .firewall-status').html('保护中');
    }else{
        $('.route-firewall-input').eq(3).attr('checked', false);
        $('.password-firewall .firewall-status').html('已关闭');
    }
    if(i==4){
        $('.route-firewall-status').html('已全部开启');
    }
    if(i<4 && i>0){
        $('.route-firewall-status').html('已部分开启');
    }
    if(i==0){
        $('.route-firewall-status').html('已全部关闭');
    }
}
function firewallSwitch(data) {
    resultTipshow(-1,-1);
    $.ajax({
        type: 'POST',
        url: '/cgi-bin/web.cgi/',
        data: JSON.stringify(data),
        dataType: 'json',
        success: function (res) {
            if (res.errorcode == 0) {
                resultTipshow(1,'修改成功');
                firewallStatusUpdata(data);
                getfirewallstatus();
            }else {
                err.error_msg(res);
            }
        }
    })
}
$(".qos-icon").parent().click(function () {
    getQosInfo();
})
var qosdownload,qosupload;
function getQosInfo() {
    var data={
        "requestUrl": "proc_qos_cfg",
        "method": "get"
    }
    postAjax(data,qos_info_fill)
}
function qos_info_fill(res) {
    if (res.errorcode == 0) {
        qosdownload = (res.data.download/1024).toFixed(2);
        qosupload = ((res.data.upload).toFixed(2)/1024).toFixed(2);
        $('.download-boradband').val(qosdownload);
        $('.upload-boradband').val(qosupload);
        if(res.data.enabled==1) {
            $('.qos-switch-input').attr('checked','checked');
            $('.Qos-btn-group button').removeAttr('disabled');
            $('.load-speed input').removeAttr('disabled');
        }else {
            $('.qos-switch-input').attr('checked',false);
            $('.Qos-btn-group button').attr('disabled','disabled');
            $('.load-speed input').attr('disabled','disabled');
        }
        var i=Number(res.data.qosmode);
        if(!($('.Qos-btn').eq(i).children().first().hasClass('selected'))){
            $('.Qos-btn .selected').removeClass('selected');
            $('.Qos-btn').eq(i).children().first().addClass('selected');
        }
    }
}
$('.boradband-test').click(function () {
    speedTestingStart();
})
var speedTestingSwitch;
function speedTestingStart() {
    // $('.boradband-testing').toggleClass('hide');
    removeShadow('.boradband-testing')
    var data={
        "data":{
            "action":1
        },
        "requestUrl": "proc_wan_speedtest_cfg",
        "method": "post"
    }
    postAjax(data,speed_test,speedtest_error)

}
function speed_test(res) {
    if(res.errorcode==0){
        speedtesting()
        speedTestingSwitch=1
    }else {
        err.error_msg(res);
    }
}
function speedtest_error() {
    resultTipshow(0,'测速出错请检查网络后重试')
}
var speedTestingInterval;
function speedtesting() {
    var data={
        "requestUrl": "proc_wan_speedtest_cfg",
        "method": "get"
    }
    postAjax(data,get_testing_spee)
}
function get_testing_spee(res) {
    if(res.errorcode==0){
        // clearTimeout(speedTestingInterval)
        if(res.data.speedtest==0){
            speedTestingInterval=setTimeout(function () {
                if(speedTestingSwitch==1){
                    speedtesting();
                }
            },300);
        }
        if(res.data.speedtest==1){
            $('.testing-result').html(Number(res.data.download).toFixed(2)+'Mbps');
            // $('.testing-result-upload').html(Number(res.data.upload).toFixed(2)+'Mbps');
            // $('.boradband-testing').addClass('hide');
            $('.boradband-testing').hide()
            $('.popup-shadow').addClass('hide');
            // $('.speed-testing-results').removeClass('hide');
            removeShadow('.speed-testing-results')
            $('.testing-tip>span').html('测试中请稍候，此过程需要1-2分钟左右')
        }
    }else {
        err.error_msg(res);
    }
}
$('.testing-cancel').click(function () {
    speedTestingSwitch=0;
    // $('.boradband-testing').addClass('hide');
    addShadow('.boradband-testing')
    var data={
        "data":{
            "action":0
        },
        "requestUrl": "proc_wan_speedtest_cfg",
        "method": "post"
    }
   postAjax(data,speed_test_cancel)
})
function speed_test_cancel(res) {
    if(res.errorcode==0){
        clearTimeout(speedTestingInterval);
        // speedTestingSwitch=1;
    }else {
        err.error_msg(res);
    }
}
$('.restart-testing').click(function () {
    // $(speed-testing-results).addClass('hide')
    addShadow('.speed-testing-results')
    speedTestingStart();
})
$('.reset-speed').click(function () {
    // $('.speed-testing-results').addClass('hide');
    addShadow('.speed-testing-results')
    // $('.boradband-setting').toggleClass('hide');
})
$('.result-confirm').click(function () {
    // $('.speed-testing-results').addClass('hide');
    addShadow('.speed-testing-results')
})
$('.download-unit').on('input',function () {
    var val=$(this).val()
    // if(val.lastIndexOf('.')!=-1){
    //     if(val.lastIndexOf('.')!=val.indexOf('.')){
    //         $(this).val(val.replace(/\,/g,''))
    //     }
    // }
    // if(val.indexOf('.')!=-1){
    //     var m=val.split('.')[0].replace(/\D/g,'');
    //     var n=val.split('.')[1].replace(/\D/g,'');
    //     $(this).val(m+'.'+n)
    // }else {
    //     $(this).val(val.replace(/\s/g,''))
    // }
    var l=(val.split('.')).length-1;
    if(l>1){
        errorShow('.download-unit','格式错误');
    }else {
        errorhide();
    }
})
$('.upload-unit').on('input',function () {
    var val=$(this).val()
    // if(val.lastIndexOf('.')!=-1){
    //     if(val.lastIndexOf('.')!=val.indexOf('.')){
    //         $(this).val(val.replace(/\,/g,''))
    //     }
    // }
    // if(val.indexOf('.')!=-1){
    //     var m=val.split('.')[0].replace(/\D/g,'');
    //     var n=val.split('.')[1].replace(/\D/g,'');
    //     $(this).val(m+'.'+n)
    // }else {
    //     $(this).val(val.replace(/\s/g,''))
    // }
    var l=(val.split('.')).length-1;
    if(l>1){
        errorShow('.upload-unit','格式错误');
    }else {
        errorhide();
    }
})
$('.manual-setting-content input').on('input',function () {
    if($('.tip-info').html().length>0){
        $('.tip-info').removeClass('red')
        $('.tip-info').html('')
    }
})
$('.qos-save').click(function () {
    if($('.qos-switch-input').is(":checked")){
        var n=1;
        if($('.Qos-btn .selected').length==0){
            errorShow('.last-btn','Qos模式不能为空');
            return
        }
    }else {
        var n=0;
    }
    var download=$('.download-boradband').val();
    var upload=$('.upload-boradband').val();
    var reg=(/(^[0]{1}[\.]{0,1}[0-9]{0,2}$)|(^[1-9]{1}[0-9]{0,3}[\.]{0,1}[0-9]{0,2}$)/);
    if(download.length==0){
        errorShow('.download-unit','下行带宽不能为空');
        return
    }
    if(upload.length==0){
        errorShow('.upload-unit','上行带宽不能为空');
        return
    }
    if(!reg.test(download)){
        errorShow('.download-unit','请输入0-1000以内的数值,最多2位小数');
        return
    }
    if(!reg.test(upload)){
        errorShow('.upload-unit','请输入0-1000以内的数值,最多2位小数');
        return
    }
    if(download<0 || download>1000){
        errorShow('.download-unit','0代表不限速，最大值为1000');
        return
    }
    if(upload<0 || upload>1000){
        errorShow('.upload-unit','0代表不限速，最大值为1000');
        return
    }
    resultTipshow(-1,-1);
    var data={
        "data": {
            "qosmode":$('.Qos-btn .selected').parent().index(),
            "enabled": n,
            "download": Number(download*1024),
            "upload": Number(upload*1024),
        },
        "requestUrl": "proc_qos_cfg",
        "method": "put"
    }
   postAjax(data,qos_set)
})
function qos_set(res) {
    if(res.errorcode==0){
        $('.boradband-setting').addClass('hide');
        getQosInfo();
        resultTipshow(1,'保存成功');
    }else {
        err.error_msg(res);
    }
}

$('.qos-switch').click(function () {
    if($('.qos-switch-input').is(":checked")){
        $('.Qos-btn-group button').removeAttr('disabled');
        $('.load-speed input').removeAttr('disabled');
    }else {
        $('.Qos-btn-group button').attr('disabled','disabled');
        $('.load-speed input').attr('disabled','disabled');
        $('.download-boradband').val(qosdownload);
        $('.upload-boradband').val(qosupload);
    }
})
$('.Qos-btn button').click(function () {
    if(!$(this).hasClass('selected')){
        $('.Qos-btn .selected').removeAttr('class');
        $(this).attr('class','selected');
    }else {
        $(this).removeAttr('class');
    }
})

$('.wireless-repeater-icon').parent().click(function () {
    getwdsstatus();
})
var wdsstatus;
function getwdsstatus(e) {
    var data={
        "requestUrl": "get_wds_cfg",
        "method": "get"
    }
    $.ajax({
        type:'POST',
        url:'/cgi-bin/web.cgi/',
        data:JSON.stringify(data),
        datatype:'json',
        success:function (res) {
            if(res.errorcode==0){
                wdsstatus=res.data;
                var enable=res.data.enable;
                // repeaterStatus(enable);
                if(enable==1){
                    // $('.repeater-content').removeClass('hide');
                    $('.nearly-btn').removeAttr('disabled');
                    if(res.data.status==0 && typeof(e)=='undefined' ){
                        wispInterval
                    }
                }
                else {
                    if(enable==0){
                        clearTimeout(wispInterval);
                    }
                    // $('.repeater-content').addClass('hide');
                    $('.nearly-btn').attr('disabled','disabled');
                    $('.nearly-table').addClass('hide');
                    infolist(3);
                    $('.repeater-status-content .disabled').addClass('hide')
                }
                // res.data.auto_switch==1?$(".wireless-insert-input").attr('checked',true):$(".wireless-insert-input").removeAttr('checked');
                if(e>0){
                    if(e==150){
                        res.data.status==1?checkNet(res.data):$('.repeater-connect-status').html('连接失败');
                    }else {
                        res.data.status==1?checkNet(res.data):$('.repeater-connect-status').html('正在连接...');
                    }
                }else {
                    $(".repeater-net-input").attr('checked',enable==1?true:false);
                    res.data.status==1?$('.repeater-connect-status').html('已连接'):$('.repeater-connect-status').html('未连接');
                }
                $('.repeater-connect-ssid').html(res.data.ssid);
            }else {
                err.error_msg(res);
            }
        }
    })
}
function checkNet(res) {
    if(res.isSameNet==1){
        changelanip(res.suggestLanIp)
    }
    clearTimeout(wispInterval);
    $('.repeater-connect-status').html('连接成功')
    $('.repeater-status-content .disabled').addClass('hide')
}
function changelanip(ip) {
    $('.result-icon').removeClass('loading-img');
    resultShowIn('success-icon')
    var t=Math.random()
    $('.result-tip').html('连接成功，检测到被中继网关与当前网关LAN侧ip地址相同，当前网关LAN侧ip地址修改为'+ip+'，有线连接的网络请插拔网线或者重启网卡，无线连的网络接重新连接后，获取新的ip地址，');
    $('.result-content').append("<a href='http://"+ip+"/login.html?t="+t+"' target='_self'>点击跳转到登录页面</a>")
}
$(".repeater-net-input").next().click(function(){
    if ($(".repeater-net-input").is(':checked')) {
        // $(".repeater-content").removeClass('hide');
        // $(".repeater-content").fadeIn();
        $('.nearly-btn').removeAttr('disabled');
        // $(".nearby-wireless-networks").fadeIn();
    } else{
        clearTimeout(wispInterval);
        // $(".repeater-content").fadeOut();
        // $(".repeater-content").addClass('hide');
        $('.nearly-btn').attr('disabled','disabled');
        // $(".nearby-wireless-networks").fadeOut();
    }
});
$('.repeater-save').click(function () {
    repeaterStatus();
})
function repeaterStatus() {
    var enable=($('.repeater-net-input').is(":checked")==true?1:0);
    if(enable==1){
        if(wdsstatus.ssid.length==0){
            wifiScan()
        }else {
            wdsclose(enable)
        }
    }else {
        wdsclose(enable)
    }
}
function wdsclose(enable) {
    var data={
        "data": {
            "enable":enable
        },
        "requestUrl": "set_wds_cfg",
        "method": "post"
    }
    resultTipshow(-1,-1);
   postAjax(data,wds_close)
}
function wds_close(res) {
    if(res.errorcode==0){
        $('.nearly-table').addClass('hide');
        infolist(3);
        resultTipshow(1,'保存成功');
        getwdsstatus();
    }else {
        err.error_msg(res);
    }
}
var wispInterval;
function refreshstatus(e) {
    if(!e){
        var e=1;
    }
    clearTimeout(wispInterval)
    wispInterval=setTimeout(function () {
        getwdsstatus(e);
        if(e==1){
            totop();
            resultTipshow(-1,'请注意wifi连接状态，正在连接...');
            $('.repeater-status-content .disabled').removeClass('hide')
            setTimeout(function () {
                $('.result-content').hide();
                $('.popup-shadow').addClass('hide');
            },2000)
        }
        if(wdsstatus.status!=1){
            if(e<150){
                e++;
                refreshstatus(e);
            }else {
                clearTimeout(wispInterval);
                $('.repeater-status-content .disabled').addClass('hide')
                $('.repeater-connect-status').html('连接失败');
            }
        }else {
            clearTimeout(wispInterval);
            $('.repeater-status-content .disabled').addClass('hide')
            $('.repeater-connect-status').html('连接成功');
        }
    },1000)
}
$(document).on('click','.tip-close',function () {
    $('.tipshow').fadeOut()
    $('.result-tip').html('');
    $('.result-content').remove($html);
})
var repeaterInfo='';
var pagerow=0;
var pagenum=0;
var rowcount=0;
var pagecount=0;
$('.nearly-btn').click(function () {
   wifiScan()
})
function wifiScan() {
    var data={
        "requestUrl": "get_wds_wifi_scan",
        "method": "get"
    }
    resultTipshow(-1,'正在搜索，请稍候...');
    postAjax(data,wifi_scan)
}
var listinfo=[];
function wifi_scan(res) {
    if(res.errorcode==0 && res.data.wifi.length>0){
        resultTipshow(1,'搜索完成');
        var wifilistinfo=res.data.wifi;
        for(var i=0;i<wifilistinfo.length;i++){
            if(wdsstatus.mac==wifilistinfo[i].mac){
                wifilistinfo[i].status=wdsstatus.status;
            }else {
                wifilistinfo[i].status=-1
            }
        }
        function sortstatus(a,b){
            if(a.status == b.status){
                return (a.power < b.power) ? 1 : -1
            }else {
                return (a.status < b.status) ? 1 : -1
            }
        }
        listinfo=wifilistinfo.sort(sortstatus)

        //     .sort(function(a,b){
        //     return (a.power < b.power) ? 1 : -1
        // });
        infolist(3)
        $('.nearly-table').removeClass('hide');
    }else {
        if(res.data.wifi.length==0){
            wifiScan
        }else {
            err.error_msg(res);
        }
    }
}
var template_list_id,tbody_class;
function infolist(n) {
    var i=listinfo.length;
    for(var m=0;m<i;m++){
        listinfo[m].index=m+1;
    }
    var table_name;
    switch (n){
        case 0:
            table_name='.anti-device-manage ';
            template_list_id='antiListTemplate';
            tbody_class='.anti-device-list';
            break;
        case 1:
            table_name='.children-table ';
            template_list_id='childListTemplate';
            tbody_class='.children-list';
            break;
        case 2:
            table_name='.wifi-interval-list ';
            template_list_id='intervalListTemplate';
            tbody_class='.wifi-interval-table';
            break;
        case 3:
            table_name='.nearly-table ';
            template_list_id='wirelessListTemplate';
            tbody_class='.repeater-info';
            break;
        case 4:
            table_name='.vps-table ';
            template_list_id='vpsListTemplate';
            tbody_class='.portmapping-info';
            break;
        default:
            break;
    }
    if(i>10){
        // var infoarr=new Array();
        // infoarr=[];
        // for(var j=0;j<10;j++){
        //     infoarr.push(listinfo[j])
        // }
        // repeaterInfo = infoarr;
        $(table_name+'tfoot').removeClass('hide');
    }else {
        $(table_name+'tfoot').addClass('hide');
        // repeaterInfo = listinfo;
    }
    pagerow=10;
    pagenum=1;
    rowcount=i;
    pagecount=i % pagerow==0 ? i/pagerow : Math.floor(i/pagerow)+1;
    listpage();
    displayRow(0,9)
}
function displayRow(m,n) {
    var i=listinfo.length;
    if((i-m)<10){
        var k=i;
    }else {
        var k=n+1;
    }
    var infoarr=new Array();
    infoarr=[];
    for(var j=m;j<k;j++){
        infoarr.push(listinfo[j])
    }
    template_list(infoarr);
}
function listpage(){
    $(".frame-list .show .pagerow").html(pagerow);
    $(".frame-list .show .pagenum").html(pagenum);
    $(".frame-list .show .pagecount").html(pagecount);
    $(".frame-list .show .rowcount").html(rowcount);
    if (pagenum == 1) {
        $(".frame-list .show .fpbtn").attr("disabled", true);
        $(".frame-list .show .rpbtn").attr("disabled", true);
    }else {
        $(".frame-list .show .fpbtn").removeAttr("disabled");
        $(".frame-list .show .rpbtn").removeAttr("disabled");
    }
    if (pagenum == pagecount) {
        $(".frame-list .show .npbtn").attr("disabled", true);
        $(".frame-list .show .lpbtn").attr("disabled", true);
    }else {
        $(".frame-list .show .npbtn").removeAttr("disabled");
        $(".frame-list .show .lpbtn").removeAttr("disabled");
    }
    $(".frame-list .show .gpinput").val(pagenum);
}
function firstpage(){
    pagenum=1;
    displayRow(0,pagerow-1);
    listpage();
}
function previouspage(){
    pagenum--;
    displayRow((pagenum-1)*10,pagenum*10-1);
    listpage();
}
function nextpage(){
    if ((pagenum + 1) >= pagecount)
    {
        pagenum=pagecount;
        displayRow((pagenum-1)*10,rowcount-1);
    } else {
        pagenum++;
        displayRow((pagenum-1)*10,pagenum*10-1);
    }
    listpage();
}
function lastpage(){
    pagenum=pagecount;
    displayRow((pagenum-1)*10,rowcount-1);
    listpage();
}
function jumppage(){
    var t = $(".frame-list .show .gpinput").val();
    var re = /^[1-9]+[0-9]*$/;
    if (t == null || t == undefined || t == '') {
        alert("请输入跳转页数!");
        $(".gpinput").focus();
        return;
    }
    if (!re.test(t)) {
        alert("请输入正确跳转页数!");
        $(".frame-list .show .gpinput").focus();
        return;
    }
    if(t==pagenum){
        alert('已是当前页');
    }else {
        if (t > pagecount)
        {
            $(".frame-list .show .gpinput").val(pagecount)
            t = pagecount;
            pagenum=t;
            displayRow((pagenum-1)*10,rowcount-1);
        }else {
            $(".frame-list .show .gpinput").val(t)
            pagenum=t;
            displayRow((pagenum-1)*10,pagenum*10-1);
        }
    }
    listpage();
}
function template_list(data) {
    var compiled = _.template(document.getElementById(template_list_id).innerHTML);
    var str = compiled(data);
    $(tbody_class).html(str);
}
var connectssid='',connectmac='',connectmode='', connectencrypt='',noencrypt;
function repeaterConnect(a,b,c,d) {
    connectssid=a;
    $('.repeater-wifi-ssid').val(connectssid);
    if(d=='none'){
        noencrypt=1;
        $('.repeater-wifi-pwd').parent().hide();
        $('.repeater-wifi-pwd').attr('disabled','disabled');
    }else {
        noencrypt=0;
        $('.repeater-wifi-pwd').parent().removeAttr('style');
        $('.repeater-wifi-pwd').removeAttr('disabled','disabled');
    }
    // $('.repeater-edit-info').removeClass('hide');
    removeShadow('.repeater-edit-info')
    connectmac=b;
    connectmode=c;
    connectencrypt=d;
}
// $(document).on('click','.repeater-disconnect',function () {
//     var ssid=$(this).children().html();
//     $('.repeater-wifi-ssid').val(ssid);
//     $('.repeater-edit-info').removeClass('hide');
//     connectencrypt=$(this).parent().parent().children().eq(4).html();
// })
$('.repeater-edit-info .close-icon').click(function () {
    var pwd=$('.repeater-wifi-pwd').val('');
    $('.repeater-wifi-pwd').parent().removeAttr('style');
    $('.repeater-wifi-pwd').removeAttr('disabled','disabled');
})
$('.repeater-confirm').click(function () {
    if(noencrypt!=1){
        var pwd=$('.repeater-wifi-pwd').val();
        var tip=$('.repeater-tip-info');
        if(pwd.length==0){
            tip.html('请输入密码');
            return;
        }
        if(cn_test(pwd)==-1){
            tip.html("密码不支持中文字符");
            return
        }
        if(!(/^.{8,63}$/).test(pwd)){
            tip.html('请输入8-63位密码');
            return;
        }
    }else {
        noencrypt=0;
        $('.repeater-wifi-pwd').removeAttr('disabled','disabled');
    }

    // if(mode==null){
    //     resultTipshow(0,'wifi不存在');
    //     return;
    // }
    $('.repeater-edit-info').hide();
    resultTipshow(-1,-1);
    var data={
        "data": {
            "enable":1
        },
        "requestUrl": "set_wds_cfg",
        "method": "post"
    }
   postAjax(data,repeater_set)
})
function repeater_set(res) {
    if(res.errorcode==0){
        repeaterconnect()
    }else {
        err.error_msg(res);
    }
}
function repeaterconnect() {
    var data={
        "data": {
            "ssid":connectssid,
            "mac":connectmac,
            "password":$('.repeater-wifi-pwd').val(),
            "mode":Number(connectmode),
            "encrypt":connectencrypt
        },
        "requestUrl": "set_up_wds",
        "method": "post"
    }
    postAjax(data,repeater_connect)
}
function repeater_connect(res) {
    if(res.errorcode==0){
        // $('.repeater-edit-info').addClass('hide');
        resultTipshow(1,'保存成功');
        $('.nearly-table').addClass('hide');
        infolist(3);
        $('.repeater-wifi-ssid').val('');
        $('.repeater-wifi-pwd').val('');
        wdsstatus.status=0;
        setTimeout(function () {
            refreshstatus();
        },0)
    }else {
        err.error_msg(res);
    }
}
$(document).on('click','.repeater-edit',function () {
    var ssid=$(this).children().html();
    $('.repeater-wifi-ssid').val(ssid);
    // $('.repeater-edit-info').removeClass('hide');
    removeShadow('.repeater-edit-info')
    // $('.repeater-edit-info').fadeIn()
})



$('.vpn-setting-icon').parent().click(function () {
    getvpnlist(-1);
})
function getvpnlist(i,m) {
    var data={
        "requestUrl": "get_vpn_cfg",
        "method": "get"
    }
    $.ajax({
        type:'POST',
        url:'/cgi-bin/web.cgi/',
        data:JSON.stringify(data),
        datatype:'json',
        success:function (res) {
            if (res.errorcode == 0) {
                $vpn_list= res.data.vpn_list;
                var data = $vpn_list;
                if(data.length==0){
                   // $('.vpn-table').append('<div class="none-interval"><span>暂未添加VPN</span></div>')
                    data=[{"length":0}]
                }else {
                    if(i>=-1){
                        var n=0,q=0,k;
                        for(var j=0;j<data.length;j++){
                            if(data[j].ifname==m){
                                n++;
                                k=j;
                            }
                            if(data[j].enable==1){
                                q++;
                                k=j;
                                m=data[j].ifname;
                            }
                        }
                        if(n==1 || q==1){
                            if(i<39){
                                if(data[k].status=='up'){
                                    clearTimeout(vpnrefreshInterval)
                                }else {
                                    data[k].status='0';
                                    i++;
                                    vpnrefresh(i,m);
                                }
                            }else{
                                if(data[k].status=='down'){
                                    data[k].status='-1'
                                }
                                clearTimeout(vpnrefreshInterval)
                            }
                        }else {

                            clearTimeout(vpnrefreshInterval)
                        }
                    }
                }
                var compiled = _.template(document.getElementById('vpnListTemplate').innerHTML);
                var str = compiled(data);
                $('.vpn-info').html(str);
            }else {
                err.error_msg(res);
            }
        }
    })
}
$('.pwd-switch').click(function () {
    $(this).toggleClass('hidePassword');
    $(this).toggleClass('showPassword');
    if($(this).prev().attr('type')=='password'){
        $(this).prev().attr('type','text')
    }else {
        $(this).prev().attr('type','password')
    }
})
var pwdststus=0;
$('.vpn-setting-save').click(function () {
    vpncheck(1);
})
function vpncheck(m) {
    if ($('.vpn-name').val().replace(/(^\s*)|(\s*$)/g, "").length == 0) {
        errorShow('.vpn-name', 'vpn描述不能为空')
        return;
    }
    if ($('.vpn-name').val().length > 16) {
        errorShow('.vpn-name', '请输入1-16位vpn描述')
        return;
    }
    if($('.vpn-tpye').val().length==0){
        errorShow('.vpn-tpye', '请选择vpn协议类型')
        return;
    }
    var vpn = $('.vpn-server-ip').val();
    if (spacecheck(vpn) == -1) {
        errorShow('.vpn-server-ip', 'vpn服务器地址不支持空格')
        return;
    }
    if (vpn.length > 63 || vpn.length == 0) {
        // errorShow('.vpn-server-ip', '请输入1-63位vpn服务器地址')
        errorShow('.vpn-server-ip', '请输入vpn服务器地址')
        return;
    }
    if((/[\'\"\<\>\(\)\,\;\+\[\]\{\}]+/g).test(vpn)){
        // errorShow('.vpn-server-ip', "vpn服务器地址不能包含非法字符'&quot;<>(),;+[]{}");
        errorShow('.vpn-server-ip', '服务器地址错误')
        return;
    }
    var serverReg=/^[a-zA-Z0-9][-a-zA-Z0-9]{0,62}(\.[a-zA-Z0-9][-a-zA-Z0-9]{0,62})+$/;
    var ipReg=/^((?:(?:25[0-5]|2[0-4]\d|((1\d{2})|([1-9]?\d)))\.){3}(?:25[0-5]|2[0-4]\d|((1\d{2})|([1-9]?\d))))$/;
    if((/[^\d\.]+/g).test(vpn)){
       if (!serverReg.test(vpn)) {
            // errorShow('.vpn-server-ip', '请输入正确的域名')
           errorShow('.vpn-server-ip', '服务器地址错误')
            return;

        }
    }else {
        if (!ipReg.test(vpn)) {
            errorShow('.vpn-server-ip', '服务器地址错误')
            // vpnipcheck('.vpn-server-ip');
            return;
        }
    }
    var username = $('.vpn-username').val()
    if (spacecheck(username) == -1) {
        errorShow('.vpn-username', 'vpn用户名不支持空格')
        return;
    }
    if (cn_test($('.vpn-username').val()==-1)) {
        errorShow('.vpn-username', "vpn用户名不支持中文字符");
        return
    }
    if (username.length > 32 || username.length == 0) {
        errorShow('.vpn-username', '请输入1-32位vpn用户名')
        return;
    }
    if(m==1){
        if($('.vpn-pwd').val().length == 0 || $('.vpn-pwd').val().length > 32) {
            errorShow('.vpn-pwd', '请输入1-32位vpn密码')
            return;
        }
        if(cn_test($('.vpn-pwd').val()==-1)){
            errorShow('.vpn-pwd',"vpn密码不支持中文字符");
            return
        }
    }
    if(m==0){
        if ($('.vpn-pwd').val().length > 32) {
            errorShow('.vpn-pwd', '请输入1-32位vpn密码')
            return;
        }
        if ($('.vpn-pwd').val().length > 0) {
           if(cn_test($('.vpn-pwd').val()==-1)){
                errorShow('.vpn-pwd',"vpn密码不支持中文字符");
                return
            }
        }
    }
    resultTipshow(-1,-1);
    var passencrypted='';
    if(pwdststus==1 ){
        vpnModify(passencrypted);
    }else {
        if( $('.vpn-pwd').val().length>0){
            var data = {
                "requestUrl": "getkeycfg",
                "method": "get"
            };
            $.ajax({
                type: 'POST',
                url: '/cgi-bin/web.cgi/',
                data: JSON.stringify(data),
                dataType: 'json',
                success: function (res) {
                    if (res.errorcode == 0) {
                        var encrypt = new JSEncrypt();
                        var publickey = res.data.rand_key;
                        encrypt.setPublicKey(publickey);
                        passencrypted = encrypt.encrypt($('.vpn-pwd').val());
                        if(m==1){
                            var data = {
                                "data": {
                                    "description": $('.vpn-name').val(),
                                    "mode": ($('.vpn-tpye').val()),
                                    "server_ip": $('.vpn-server-ip').val(),
                                    "username": $('.vpn-username').val(),
                                    "password": passencrypted,
                                },
                                "requestUrl": "add_vpn_cfg",
                                "method": "post"
                            }
                            postAjax(data,encryptsubmit);
                        }
                        if(m==0){
                            vpnModify(passencrypted);
                        }
                    } else {
                        err.error_msg(res);
                    }
                }
            })
        }
    }
}
function vpnipcheck(m) {
    var iparr=$(m).val().split('.');
    if(iparr.length<4){
        errorShow(m,'ip地址请输入完整');
        return;
    }
    for(var i=0;i<iparr.length;i++){
        if((/[^\d]+/g).test(iparr[i])){
            errorShow(m,'第'+(i+1)+'段ip地址存在非数字字符');
            return;
        }
        if(iparr[i].length>3){
            errorShow(m,'第'+(i+1)+'段ip值长度超出3位');
            return;
        }
        if(iparr[i]>255 || iparr[i]<0){
            errorShow(m,'第'+(i+1)+'段ip值只能在0-255之间');
            return;
        }
        if(iparr[i].length==0){
            errorShow(m,'第'+(i+1)+'段ip地址请输入完整');
            return;
        }
    }
}
function encryptsubmit(res) {
    if (res.errorcode == 0) {
        vpnSettingClear();
        getvpnlist();
        resultTipshow(1,'保存成功');
    }else {
        err.error_msg(res);
    }
}
var modifyvpnifname='';
function vpnEdit(a,b,c,d,e) {
    errorhide()
    $('.vpn-name').val(b);
    $('.vpn-tpye').val(c.toUpperCase());
    $('.vpn-server-ip').val(d);
    $('.vpn-username').val(e);
    $('.vpn-pwd').val('********');
    pwdststus=1;
    // $('.vpn-pwd').val($(this).parent().parent().children().eq(5).html());
    modifyvpnifname=a;
    $('.vpn-setting-save').addClass('hide');
    $('.vpn-btn .modify-btn').removeClass('hide');
}
$('.vpn-pwd').on('focus',function () {
    if(pwdststus==1){
        $('.vpn-pwd').val('');
    }
})
$('.vpn-pwd').on('blur',function () {
    if(pwdststus==1 || pwdststus==2){
        if($('.vpn-pwd').val().length==0){
            $('.vpn-pwd').val('********');
            pwdststus=1;
        }else {
            pwdststus=2;
        }
    }

})
$('.vpn-setting-modify').click(function () {
    vpncheck(0);
})
$('.vpn-modify-cancel').click(function () {
    vpnClear();
    // $('.vpn-btn .modify-btn').addClass('hide')
    // $('.vpn-setting-save').removeClass('hide')
})
function vpnSettingClear(e) {
    $('.vpn-name').val('');
    $('.vpn-tpye').val('L2TP');
    $('.vpn-server-ip').val('');
    $('.vpn-username').val('');
    $('.vpn-pwd').val('');
    if(e==2){
        $('.modify-btn').addClass('hide')
        $('.vpn-setting-save').removeClass('hide')
        clearTimeout(vpnrefreshInterval);
        pwdststus=0;
    }
}
function vpnModify(passencrypted) {
    var data={
        "data": {
            "description":$('.vpn-name').val(),
            "ifname":modifyvpnifname,
            "mode": ($('.vpn-tpye').val()).toLowerCase(),
            "server_ip":$('.vpn-server-ip').val(),
            "username":$('.vpn-username').val(),
            "password":passencrypted,
        },
        "requestUrl": "set_vpn_cfg",
        "method": "post"
    }
    postAjax(data,vpn_modify_sub)
}
function vpn_modify_sub(res) {
    if (res.errorcode == 0) {
        // clearTimeout(vpnrefreshInterval);
        vpnSettingClear(2);
        getvpnlist(0);
        resultTipshow(1,'修改成功');
        $('.vpn-setting-save').removeClass('hide')
        $('.vpn-setting-modify').addClass('hide')
    }else {
        err.error_msg(res);
    }
}
function vpnClear() {
    $('.vpn-name').val('');
    $('.vpn-tpye').val('L2TP');
    $('.vpn-server-ip').val('');
    $('.vpn-username').val('');
    $('.vpn-pwd').val('');
    // $('.vpn-set-form .pwd-switch').attr('class','pwd-switch hidePassword');
    $('.vpn-btn .modify-btn').addClass('hide');
    $('.vpn-setting-save').removeClass('hide');
    pwdststus=0;
    errorhide()
}
function vpnDel(m) {
    resultTipshow(-1,-1);
    var data={
        "data": {
            "ifname":m
        },
        "requestUrl": "del_vpn_cfg",
        "method": "post"
    }
    $.ajax({
        type:'POST',
        url:'/cgi-bin/web.cgi/',
        data:JSON.stringify(data),
        datatype:'json',
        success:function (res) {
            if (res.errorcode == 0) {
                if(vpnrefreshid==m){
                    clearTimeout(vpnrefreshInterval);
                }
                vpnClear();
                getvpnlist();
                resultTipshow(1,'删除成功');
            }else {
                err.error_msg(res);
            }
        }
    })
}

function vpnDisconnect(m) {
    resultTipshow(-1,-1);
    var data={
        "data": {
            "enable":1,
            "vpn":[
                {
                    "ifname":m,
                    "enable":0
                }
            ]
        },
        "requestUrl": "set_vpn_sw",
        "method": "post"
    }
    vpnSwitch(data,0);
}
function vpnConnect(m) {
    // $('.loading').fadeIn();
    resultTipshow(-1,-1);
    var vpn=[]
    for(var i=0;i<$vpn_list.length;i++){
        if($vpn_list[i].ifname==m){
            // $vpn_list[i].enable=1;
            var vpninfo={
                "ifname":$vpn_list[i].ifname,
                "enable":1
            }
            vpn.push(vpninfo)
        }else {
            var vpninfo={
                "ifname":$vpn_list[i].ifname,
                "enable":0
            }
            vpn.push(vpninfo)
        }
    }
    var data={
        "data": {
            "enable":1,
            "vpn": vpn
        },
        "requestUrl": "set_vpn_sw",
        "method": "post"
    }
    vpnSwitch(data,m);
}
function vpnSwitch(data,m) {
    $.ajax({
        type:'POST',
        url:'/cgi-bin/web.cgi/',
        data:JSON.stringify(data),
        datatype:'json',
        success:function (res) {
            // getvpnlist();
            if (res.errorcode == 0) {
                // $('.loading').fadeOut();
                vpnClear();
                if(m==0){
                    // if(vpnrefreshid==m){
                        clearTimeout(vpnrefreshInterval);
                    // }
                    getvpnlist(-1);
                }else {
                    clearTimeout(vpnrefreshInterval);
                    vpnrefresh(0,m)
                }
                resultTipshow(1,'修改成功');
            }else {
                // $('.loading').fadeOut();
                err.error_msg(res);
            }
        }
    })
}
var vpnrefreshInterval,vpnrefreshid,vpnrefreshenable;
function vpnrefresh(i,m) {
    clearTimeout(vpnrefreshInterval);
    if(i<40){
        if(i==0){
            vpnrefreshenable=1
        }
        vpnrefreshid=m;
        if(vpnrefreshenable==1){
            vpnrefreshInterval=window.setTimeout(function () {
                getvpnlist(i,m);
            },2000)
        }
    }
}
$('.portmapping-choose>span').click(function () {
    if(!$(this).hasClass('active')){
        var i= $('.portmapping-choose>span.active').index();
        $('.portmapping-choose>span.active').removeClass('active');
        $('.portmapping-item-content>div.hide').removeClass('hide');
        $('.portmapping-item-content>div').eq(i).addClass('hide');
        $(this).addClass('active');
    }
    if($('.error-info').html().length>0){
        errorhide();
    }
})

$('.port-mapping-icon').parent().click(function () {
    getportmappinglist();
    getlanInfo();
})
function getportmappinglist() {
    var data = {
        "requestUrl": "proc_pf_cfg",
        "method": "get"
    }
    postAjax(data,portmapping_list)
}
function portmapping_list(res) {
    if (res.errorcode == 0) {
        var data = res.data;
        if(data.length==0){
            data=[{"length":0}]
        }
        listinfo = data;
        infolist(4);
    }else {
        err.error_msg(res);
    }
}
function vpsClear(e) {
    $('.vps-name').val('');
    $('.vps-select').val('TCP');
    $('.vps-ip').val('');
    $('.extrnal-port-start').val('');
    $('.extrnal-port-end').val('');
    $('.intrnal-port-start').val('');
    $('.intrnal-port-end').val('');
    errorhide()
    if(e==2){
        $('.vps-btn .modify-btn').addClass('hide');
        $('.vps-save ').removeClass('hide');
    }
}

function getlanInfo() {
    var data={
        "requestUrl": "get_lan_cfg",
        "method": "get"
    }
   postAjax(data,laninfo)
}
function laninfo(res) {
    if (res.errorcode == 0) {
        $laninfo=res.data;
    }else {
        err.error_msg(res);
    }
}
function vpstip(m,n) {
    if(m==0){
        errorShow('.extrnal-port-end',n);
        return
    }
    if(m==1){
        errorShow('.intrnal-port-end',n);
        return
    }
}
$('.vps-input').on('input',function () {
    $(this).val($(this).val().replace(/\D/g,''));
   errorhide()
})
var portcompare= function (m,n,k) {
    var i=0;
    if (m.length == 0 || n.length == 0) {
        vpstip(k,'端口号请输入完整')
        i++
        return i;
    }
    if (m > n) {
        vpstip(k,'结束端口不能小于开始端口')
        i++
        return i;
    }
    if (m < 1 || m > 65535 || n < 1 || n > 65535) {
        vpstip(k,'请输入1-65535之间的端口')
        i++
        return i;
    }
}
function vpssub(e) {
    if ($('.vps-name').val().replace(/(^\s*)|(\s*$)/g,"").length == 0) {
        errorShow('.vps-name','虚拟服务器名称不能为空');
        return;
    }
    if ($('.vps-name').val().length>16) {
        errorShow('.vps-name','请输入1-16位虚拟服务器名称');
        return;
    }
    var ip = $('.vps-ip').val();
    if (ip.replace(/(^\s*)|(\s*$)/g,"").length == 0) {
        errorShow('.vps-ip','请输入内网主机ip地址');
        return;
    }
    var check = (/^((?:(?:25[0-5]|2[0-4]\d|((1\d{2})|([1-9]?\d)))\.){3}(?:25[0-5]|2[0-4]\d|((1\d{2})|([1-9]?\d))))$/);
    var ip_filter = check.test(ip);
    if (!ip_filter) {
        errorShow('.vps-ip','内网主机ip地址输入错误');
        return;
    }
    var m = $laninfo.ipaddress.split('.');
    var n = $laninfo.netmask.split('.');
    var k = ip.split('.');
    for (var i = 0; i < k.length; i++) {
        var ipPart = (m[i]*1) & (n[i]*1);
        if ((k[i] !== ipPart + "") && (!!(n[i]*1))) {
            errorShow('.vps-ip','内网主机ip地址应位LAN网段内的ip');
            return;
        }
    }
    if(m[3]==k[3]){
        errorShow('.vps-ip','内网主机ip地址不能与网关ip相同');
        return;
    }
    var m=portcompare($('.extrnal-port-start').val().replace(/(^\s*)|(\s*$)/g,""), $('.extrnal-port-end').val().replace(/(^\s*)|(\s*$)/g,""),0);
    if(m>0){
        return;
    }
    var n=portcompare($('.intrnal-port-start').val().replace(/(^\s*)|(\s*$)/g,""), $('.intrnal-port-end').val().replace(/(^\s*)|(\s*$)/g,""),1);
    if(n>0){
        return;
    }
    if ($('.extrnal-port-end').val() - $('.extrnal-port-start').val() != $('.intrnal-port-end').val() - $('.intrnal-port-start').val()) {
        vpstip(0,'内部端口号长度应该等于外部端口号长度')
        return;
    }
    resultTipshow(-1,-1);
    if(e==1){
        var data={
            "data": {
                "name":$('.vps-name').val(),
                "proto":$('.vps-select').val(),
                "srcdport":$('.extrnal-port-start').val()+':'+$('.extrnal-port-end').val(),
                "destip":$('.vps-ip').val(),
                "destport":$('.intrnal-port-start').val()+':'+$('.intrnal-port-end').val(),
                "enabled":1
            },
            "requestUrl": "proc_pf_cfg",
            "method": "post"
        }
    }
    if(e==2){
        var data={
            "data": {
                "name":oldvpsinfo[0],
                "proto":oldvpsinfo[2],
                "srcdport":oldvpsinfo[3],
                "destip":oldvpsinfo[1],
                "destport":oldvpsinfo[4],
                "enabled":1,
                "newname":$('.vps-name').val(),
                "newproto":$('.vps-select').val(),
                "newsrcdport":$('.extrnal-port-start').val()+':'+$('.extrnal-port-end').val(),
                "newdestip":$('.vps-ip').val(),
                "newdestport":$('.intrnal-port-start').val()+':'+$('.intrnal-port-end').val(),
                "newenabled":1
            },
            "requestUrl": "proc_pf_cfg",
            "method": "put"
        }
    }
    $.ajax({
        type:'POST',
        url:'/cgi-bin/web.cgi/',
        data:JSON.stringify(data),
        datatype:'json',
        success:function (res) {
            if (res.errorcode == 0) {
                getportmappinglist();
                resultTipshow(1,'保存成功');
                vpsClear(e);
            }else {
                err.error_msg(res);
            }
        }
    })
}
$('.vps-save').click(function () {
    vpssub(1,this);
})
var oldvpsinfo=[];
function vpsEdit(a,b,c,d,e) {
    oldvpsinfo[0]=a;
    oldvpsinfo[1]=b;
    oldvpsinfo[2]=c;
    oldvpsinfo[3]=d;
    oldvpsinfo[4]=e;
    $('.vps-name').val(oldvpsinfo[0]);
    $('.vps-ip').val(oldvpsinfo[1]);
    $('.vps-select').val(oldvpsinfo[2]);
    var extrnalport= d.split(":");
    $('.extrnal-port-start').val(extrnalport[0]);
    $('.extrnal-port-end').val(extrnalport[1]);
    var intrnalport= e.split(":");
    $('.intrnal-port-start').val(intrnalport[0]);
    $('.intrnal-port-end').val(intrnalport[1]);
    $('.vps-save').addClass('hide')
    $('.vps-btn .modify-btn').removeClass('hide')
}
$('.vps-modify-update').click(function () {
    vpsModify();
})
$('.vps-modify-cancel').click(function () {
    vpsClear(2);
})
function vpsModify() {
    vpssub(2);
}
function vpsDel(a,b,c,d,e) {
    resultTipshow(-1,-1);
    var data={
        "data": {
            "name":a,
            "destip":b,
            "proto":c,
            "srcdport":d,
            "destport":e,
            "enabled":1
        },
        "requestUrl": "proc_pf_cfg",
        "method": "delete"
    }
    postAjax(data,vps_del_sub)
}
function vps_del_sub(res) {
    if (res.errorcode == 0) {
        getportmappinglist();
        vpsClear(2);
        resultTipshow(1,'删除成功');
    }else {
        err.error_msg(res);
    }
}

$('.dmz-settings').click(function () {
    dmzinfo();
})
function dmzinfo() {
    var data={
        "requestUrl": "proc_dmz_cfg",
        "method": "get"
    }
    postAjax(data,dmz_info)
}
function dmz_info(res) {
    if (res.errorcode == 0) {
        res.data.enabled==1?($('.DMZ-input').attr('checked',true),$('.dmz-ip').removeAttr('disabled')):$('.DMZ-input').removeAttr('checked');
        $('.dmz-ip').val(res.data.destip);
    }else {
        err.error_msg(res);
    }
}
$('.dmz-switch').click(function () {
    if($('.DMZ-input').is(':checked')){
        $('.dmz-ip').removeAttr('disabled')
    }else {
        $('.dmz-ip').attr('disabled','disabled')
    }
    if($('.error-info').html().length>0){
        errorhide();
    }
})
$('.DMZ-save').click(function () {
    if($('.DMZ-input').is(':checked')){
        var enable=1;
        var ip=$('.dmz-ip').val();
        if(ip.length==0){
            errorShow('.dmz-ip','请输入DMZ主机IP地址')
            return;
        }
        var ip_filter = (/^((?:(?:25[0-5]|2[0-4]\d|((1\d{2})|([1-9]?\d)))\.){3}(?:25[0-5]|2[0-4]\d|((1\d{2})|([1-9]?\d))))$/);
        if(!ip_filter.test(ip)){
            errorShow('.dmz-ip','DMZ主机ip地址格式错误');
            return;
        }
        var m=$laninfo.ipaddress.split('.');
        var n=ip.split('.');
        for(var i=0;i<3;){
            if(m[i]!=n[i]){
                errorShow('.dmz-ip','DMZ主机ip地址应位LAN网段内的ip');
                return;
            }
            i++;
        }
        if(n[3]<$laninfo.poolstart.split('.')[3] || n[3]>$laninfo.poollimit.split('.')[3] ){
            errorShow('.dmz-ip','DMZ主机ip地址应位LAN网段内的ip');
            return;
        }
    }else {
        var enable=0;
    }
    resultTipshow(-1,-1);
    var data={
        "data": {
            "enabled": enable,
            "destip": $('.dmz-ip').val()
        },
        "requestUrl": "proc_dmz_cfg",
        "method": "put"
    }
    postAjax(data,dmz_set)
})
function dmz_set(res) {
    if (res.errorcode == 0) {
        dmzinfo();
        resultTipshow(1,'保存成功');
    }else {
        err.error_msg(res);
    }
}
$('.custom-host-icon').parent().click(function () {
    gethostsinfo();
})
function gethostsinfo() {
    var data={
        "requestUrl": "get_hosts",
        "method": "get"
    }
    postAjax(data,hosts_info_list)
}
function hosts_info_list(res) {
    if (res.errorcode == 0) {
        var data=res.data.hosts;
        var hosts='';
        if(data.length>0){
            if(data[0].ip!='' && data[0].url!=''){
                for(var i=0;i<data.length;){
                    if(hosts!=''){
                        hosts=hosts+'\n'+data[i].ip+' '+data[i].url;
                        i++;
                    }else {
                        hosts=data[i].ip+' '+data[i].url;
                        i++;
                    }
                }
            }
        }
        $('.host-info').val(hosts);
        rownum();
    }else {
        err.error_msg(res);
    }
}
$('.host-info').on('input',function () {
    rownum();
    hostserrhide();
})
$('.host-info').on('scroll', function () {
    if(!$('.hosts-err').hasClass('hide')){
        hostserrhide()
    }
    $('.row-num').scrollTop($(this).scrollTop());
})
function rownum() {
    var m='';
    var n=$('.host-info').val().split("\n").length;
    for(var i=0;i<n;){
        i++;
        if(m!=''){
            m=m+'\n'+i;
        }else {
            m=i;
        }
    }
    $('.row-num').val(m)
}
$('.hosts-err').click(function () {
    hostserrhide()
})
function hostserrhide() {
    $('.hosts-err').addClass('hide');
    $('.hosts-err').removeAttr('style');
    $('.hosts-err-info').removeAttr('style');
}
function hostserror(m,n) {
    if(m>280){
        $('.host-info').animate({scrollTop: m-280+'px'},200);
    }else {
        $('.host-info').animate({scrollTop: '0px'}, 200);
    }
    setTimeout(function () {
        $('.hosts-err').removeClass('hide');
        var i=$('.host-info').width()+8;
        $('.hosts-err-info').html(n);
        if(m>280){
            $('.hosts-err').attr('style','width:'+i+'px;top:280px;')
        }else {
            $('.hosts-err').attr('style','width:'+i+'px;top:'+m+'px;')
        }
        var j=($('.hosts-err-info').width()+7).toFixed(0);
        $('.hosts-err-info').attr('style','right:-'+j+'px;')
    },240)
}
$('.host-save').click(function () {
    // var ip=$('.hosts-ip').val();
    // var url=$('.hosts-name').val();
    var ip_check=(/^((?:(?:25[0-5]|2[0-4]\d|((1\d{2})|([1-9]?\d)))\.){3}(?:25[0-5]|2[0-4]\d|((1\d{2})|([1-9]?\d))))$/);
    var url_check = /^[a-zA-Z0-9][-a-zA-Z0-9]{0,62}(\.[a-zA-Z0-9][-a-zA-Z0-9]{0,62})+$/;
    var cn_syb_reg = /[\u3002|\uff1f|\uff01|\uff0c|\u3001|\uff1b|\uff1a|\u201c|\u201d|\u2018|\u2019|\uff08|\uff09|\u300a|\u300b|\u3008|\u3009|\u3010|\u3011|\u300e|\u300f|\u300c|\u300d|\ufe43|\ufe44|\u3014|\u3015|\u2026|\u2014|\uff5e|\ufe4f|\uffe5]/
    var hosts=$('.host-info').val().split("\n");
    var hostsdata=[];
    if(hosts[0].length>0){
        for(var i=0;i<hosts.length;){
            if(hosts[i].charAt(0)==' '){
                hostserror(20 * i, '请删除第' + (i + 1) + '行ip地址前的空格');
                $('.host-info').scrollTop(20 * (i + 1));
                return;
            }
            if(hosts[i].charAt(hosts[i].length-1)==' '){
                hostserror(20 * i, '请删除第' + (i + 1) + '行末尾的空格');
                $('.host-info').scrollTop(20 * (i + 1));
                return;
            }
            if(hosts[i].split(' ').length<2) {
                hostserror(20 * i, '第' + (i + 1) + '行格式错误');
                $('.host-info').scrollTop(20 * (i + 1));
                return;
            }
            if(hosts[i].split(' ')[0].length<7 || hosts[i].split(' ')[1].length<3 || hosts[i].split(' ').length!=2){
                hostserror(20 * i, '第' + (i + 1) + '行输入格式错误');
                $('.host-info').scrollTop(20 * (i + 1));
                return;
            }
            if(hosts[i].split(' ').length>2) {
                hostserror(20 * i, '请删除第' + (i + 1) + '行多余空格');
                $('.host-info').scrollTop(20 * (i + 1));
                return;
            }
            if(!ip_check.test(hosts[i].split(' ')[0])){
                // hostserror(20*i,'第'+(i+1)+'行ip地址输入错误');
                ipcheck(i,hosts[i].split(' ')[0]);
                $('.host-info').scrollTop(20*(i+1));
                return;
            }
            if(hosts[i].split(' ')[1]!='localhost'){
                if((/[\'\"\<\>\(\)\,\;\+\[\]\{\}]+/g).test(hosts[i].split(' ')[1])){
                    hostserror(20*i,'第'+(i+1)+"行网址不能包含非法字符'&quot;<>(),;+[]{}");
                    $('.host-info').scrollTop(20*(i+1));
                    return;
                }
                if(!url_check.test(hosts[i].split(' ')[1])){
                    hostserror(20*i,'第'+(i+1)+'行网址输入错误');
                    $('.host-info').scrollTop(20*(i+1));
                    return;
                }
            }
            if(cn_syb_reg.test(hosts[i].split(' ')[1])){
                hostserror(20*i,'请删除第'+(i+1)+'行中文标点符号');
                $('.host-info').scrollTop(20*(i+1));
                return;
            }
            var n=0;
            var m=hosts[i].split(' ')[1];
            for(var j=0;j<hosts.length;){
                if(m==hosts[j].split(' ')[1]){
                    n++;
                }
                j++
            }
            if(n>1){
                hostserror(20*i,'存在与第'+(i+1)+'行网址重复的内容');
                $('.host-info').scrollTop(20*(i+1));
                return;
            }
            hostsdata[i]=({
                "ip":hosts[i].split(' ')[0],
                "url":hosts[i].split(' ')[1],
            })
            i++;
        }
    }else {
        hostsdata=[
            {
                "url":'',
                "ip":''
            }
            ]
    }
    resultTipshow(-1,-1);
    var data={"data": {
        "hosts":  hostsdata
        },
        "requestUrl": "set_hosts",
        "method": "post"
    };
    postAjax(data,hosts_set)
})
function hosts_set(res) {
    if (res.errorcode == 0) {
        resultTipshow(1,'保存成功');
        gethostsinfo();
    }else {
        err.error_msg(res);
    }
}
function ipcheck(m,n) {
    var iparr=n.split('.');
    if(iparr.length<4){
        hostserror(20*m,'第'+(m+1)+'行ip地址请输入完整');
        return;
    }
    for(var i=0;i<iparr.length;i++){
        if((/[^\d]+/g).test(iparr[i])){
            hostserror(20*m,'第'+(m+1)+'行第'+(i+1)+'段ip值存在非数字字符');
            return;
        }
        if(iparr[i].length>3){
            hostserror(20*m,'第'+(i+1)+'段ip值长度超出3位');
            return;
        }
        if(iparr[i]>255 || iparr[i]<0){
            hostserror(20*m,'第'+(i+1)+'段ip值只能在0-255之间');
            return;
        }
        if(iparr[i].length==0){
            hostserror(20*m,'第'+(m+1)+'行第'+(i+1)+'段ip值请输入完整');
            return;
        }
    }
}
$('.clear-host').click(function () {
    hostserrhide()
    $('.host-info').val('')
    var data={"data":{
        "hosts": [
            {
                "url":'',
                "ip":''
            }
        ]
    },
        "requestUrl": "set_hosts",
        "method": "post"
    };
    postAjax(data,hosts_clear)
})
function hosts_clear(res) {
    if (res.errorcode == 0) {
        resultTipshow(1,'保存成功');
        gethostsinfo();
    }else {
        err.error_msg(res);
    }
}